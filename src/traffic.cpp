#include "traffic.h"
#include "msp.h"
#include "menu.h"
#include "util.h"

#include <math.h>

TTraffic g_traffic;

#define TRAFFIC_PERIOD_MS           1000u
#define TRAFFIC_MESSAGE_PERIOD_MS   100u      // INAV runs one MSP command per serial task cycle (100 Hz), shared with MSP_SIMULATOR
#define TRAFFIC_FRAMES_PER_MESSAGE  4         // 1 + 4 * 46 = 185 bytes: fits INAV's 192-byte MSP input buffer and 256-byte UART RX buffer
#define TRAFFIC_MAX_FRAME           (8 + 38)
#define TRAFFIC_RANGE_M             64000.0f  // INAV drops traffic beyond ADSB_LIMIT_CM
#define SITL_TCP_BASE_PORT          5760      // INAV SITL: UARTn listens on 5760 + n - 1

#define SERIAL_PORT_USB_VCP         20        // INAV serialPortIdentifier_e
#define FUNCTION_MSP                (1 << 0)  // INAV serialPortFunction_e
#define FUNCTION_RX_SERIAL          (1 << 6)
#define FUNCTION_TELEMETRY_MAVLINK  (1 << 8)

// MAVLink v1 framing; ids, lengths and CRC extras as in INAV's lib/main/MAVLink headers
#define MAVLINK_STX_V1              0xFE
#define MAVLINK_MSG_ID_HEARTBEAT    0
#define MAVLINK_HEARTBEAT_CRC       50
#define MAVLINK_MSG_ID_ADSB_VEHICLE 246
#define MAVLINK_ADSB_VEHICLE_CRC    184
#define MAV_TYPE_ADSB               27
#define MAV_AUTOPILOT_INVALID       8
#define MAV_STATE_ACTIVE            4
#define MAV_COMP_ID_ADSB            156
#define TRAFFIC_MAVLINK_SYSID       100     // anything; the FC drops only its own sysid + autopilot compid

#define ADSB_ALTITUDE_TYPE_GEOMETRIC        1
#define ADSB_FLAGS_VALID_COORDS             1
#define ADSB_FLAGS_VALID_ALTITUDE           2
#define ADSB_FLAGS_VALID_HEADING            4
#define ADSB_FLAGS_VALID_VELOCITY           8
#define ADSB_FLAGS_VALID_CALLSIGN           16
#define ADSB_FLAGS_VERTICAL_VELOCITY_VALID  128

#pragma pack(1)
struct TMAVLinkHeartbeat
{
  uint32_t customMode;
  uint8_t type;
  uint8_t autopilot;
  uint8_t baseMode;
  uint8_t systemStatus;
  uint8_t mavlinkVersion;
};

// ADSB_VEHICLE, fields in wire order (largest first)
struct TMAVLinkADSBVehicle
{
  uint32_t icao;
  int32_t lat;          //deg * 1e7
  int32_t lon;          //deg * 1e7
  int32_t altitude;     //mm
  uint16_t heading;     //cdeg, course over ground
  uint16_t horVelocity; //cm/s
  int16_t verVelocity;  //cm/s
  uint16_t flags;
  uint16_t squawk;
  uint8_t altitudeType;
  char callsign[9];
  uint8_t emitterType;
  uint8_t tslc;
};
#pragma pack()

//==============================================================
//==============================================================
static uint16_t crcAccumulate(uint8_t data, uint16_t crc)
{
  uint8_t tmp = data ^ (uint8_t)(crc & 0xff);
  tmp ^= (uint8_t)(tmp << 4);
  return (crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
}

//==============================================================
//==============================================================
void TTraffic::init()
{
  this->df_override_TCAS = XPLMFindDataRef("sim/operation/override/override_TCAS");
  this->df_num_acf = XPLMFindDataRef("sim/cockpit2/tcas/indicators/tcas_num_acf");
  this->df_modeS_id = XPLMFindDataRef("sim/cockpit2/tcas/targets/modeS_id");
  this->df_flight_id = XPLMFindDataRef("sim/cockpit2/tcas/targets/flight_id");
  this->df_lat = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/lat");
  this->df_lon = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/lon");
  this->df_ele = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/ele");
  this->df_vx = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/vx");
  this->df_vz = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/vz");
  this->df_hpath = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/hpath");
  this->df_vertical_speed = XPLMFindDataRef("sim/cockpit2/tcas/targets/position/vertical_speed");
}

//==============================================================
//==============================================================
void TTraffic::reset()
{
  this->closeSITL();
  this->state = TS_FIND_PORT;
  this->portIdentifier = -1;
  this->lastRequestTime = GetTickCount() - TRAFFIC_PERIOD_MS;
  this->targetsCount = 0;
  this->framesCount = 0;
  this->nextFrame = 0;
}

//==============================================================
//==============================================================
void TTraffic::closeSITL()
{
  if (this->sitlSerial)
  {
    delete this->sitlSerial;
    this->sitlSerial = NULL;
  }
}

//==============================================================
//==============================================================
void TTraffic::loop()
{
  if (!this->sendTraffic) return;

  uint32_t t = GetTickCount();

  switch (this->state)
  {
  case TS_FIND_PORT:
    if (t - this->lastRequestTime >= TRAFFIC_PERIOD_MS)
    {
      g_msp.sendCommand(MSP2_COMMON_SERIAL_CONFIG, NULL, 0);
      this->lastRequestTime = t;
    }
    return;

  case TS_PROBE:
    if (t - this->lastRequestTime >= TRAFFIC_PERIOD_MS) this->state = TS_FIND_PORT;  //no answer: ask again
    return;

  case TS_INJECT:
    break;

  case TS_SITL_TCP:
    if (!this->sitlSerial->IsConnected())
    {
      LOG("Traffic: SITL UART%d is not reachable over TCP (is the Telemetry feature on?)", this->portIdentifier + 1);
      this->closeSITL();
      this->state = TS_IDLE;
      return;
    }
    {
      //SITL transmits MAVLink on this port too: drain it so the socket never fills up
      unsigned char scratch[256];
      while (this->sitlSerial->ReadData(scratch, sizeof(scratch)) > 0);
    }
    break;

  default:
    return;
  }

  if (this->nextFrame >= this->framesCount)
  {
    if (t - this->lastCycleTime < TRAFFIC_PERIOD_MS) return;
    this->lastCycleTime = t;
    this->readTargets();
  }

  if (t - this->lastMessageTime < TRAFFIC_MESSAGE_PERIOD_MS) return;
  this->lastMessageTime = t;

  uint8_t message[1 + TRAFFIC_FRAMES_PER_MESSAGE * TRAFFIC_MAX_FRAME];
  message[0] = (uint8_t)this->portIdentifier;
  int length = 1;
  for (int frames = 0; frames < TRAFFIC_FRAMES_PER_MESSAGE && this->nextFrame < this->framesCount; frames++, this->nextFrame++)
  {
    //the heartbeat leads each cycle, then the targets nearest first
    length += this->nextFrame == 0 ? this->encodeHeartbeat(message + length) : this->encodeVehicle(message + length, this->targets[this->nextFrame - 1]);
  }

  //a block INAV refuses (port busy) is simply dropped: every aircraft is resent next cycle
  if (this->state == TS_INJECT)
  {
    g_msp.sendCommand(MSP2_COMMON_SERIAL_INJECT, message, length);
  }
  else
  {
    this->sitlSerial->WriteData(message + 1, length - 1);
    this->sitlSerial->flushOut();
  }
}

//==============================================================
//==============================================================
void TTraffic::onSerialConfig(const uint8_t* messageBuffer, int length)
{
  if (this->state != TS_FIND_PORT) return;

  //one entry per port: identifier(u8), functionMask(u32), 4 baud rate indexes(u8)
  for (int i = 0; i + 9 <= length; i += 9)
  {
    uint32_t functionMask;
    memcpy(&functionMask, messageBuffer + i + 1, sizeof(functionMask));
    //a free MAVLink port: not shared with MSP (MAVLink telemetry there runs only while armed)
    //or a serial receiver, and not the USB VCP
    if ((functionMask & FUNCTION_TELEMETRY_MAVLINK) && !(functionMask & (FUNCTION_MSP | FUNCTION_RX_SERIAL)) && messageBuffer[i] != SERIAL_PORT_USB_VCP)
    {
      this->portIdentifier = messageBuffer[i];
      uint8_t probe = (uint8_t)this->portIdentifier;
      g_msp.sendCommand(MSP2_COMMON_SERIAL_INJECT, &probe, 1);
      this->lastRequestTime = GetTickCount();
      this->state = TS_PROBE;
      return;
    }
  }

  LOG("Traffic: no free INAV serial port is set to MAVLink");
  this->state = TS_IDLE;
}

//==============================================================
//==============================================================
void TTraffic::onSerialInject(bool accepted)
{
  if (this->state != TS_PROBE) return;

  if (accepted)
  {
    LOG("Traffic: sending to INAV serial port %d over MSP", this->portIdentifier);
    this->state = TS_INJECT;
  }
  else if (g_menu.isSITLConnection && this->portIdentifier < 8)
  {
    //SITL UARTs are TCP sockets: an older SITL takes MAVLink straight into the port
    char ip[64];
    strncpy(ip, g_menu.SITLIP.c_str(), sizeof(ip) - 1);
    ip[sizeof(ip) - 1] = 0;
    this->sitlSerial = new TCPSerial();
    this->sitlSerial->OpenConnection(ip, SITL_TCP_BASE_PORT + this->portIdentifier);
    LOG("Traffic: sending to SITL UART%d over TCP", this->portIdentifier + 1);
    this->state = TS_SITL_TCP;
  }
  else
  {
    LOG("Traffic: INAV refused MSP2_COMMON_SERIAL_INJECT: the MAVLink port is not open (is the Telemetry feature on?) or this INAV version is too old");
    this->state = TS_IDLE;
  }
}

//==============================================================
//==============================================================
void TTraffic::readTargets()
{
  this->nextFrame = 0;
  this->framesCount = 1;  //heartbeat only
  this->targetsCount = 0;

  if (!this->df_num_acf) return;  //X-Plane older than 11.50: no TCAS datarefs

  //X-Plane's own AI fills slots 1..count-1; a plugin that overrides TCAS (LiveTraffic) may leave gaps
  bool overridden = this->df_override_TCAS && XPLMGetDatai(this->df_override_TCAS);
  int slots = overridden ? TRAFFIC_TCAS_SLOTS : XPLMGetDatai(this->df_num_acf);
  if (slots > TRAFFIC_TCAS_SLOTS) slots = TRAFFIC_TCAS_SLOTS;
  if (slots <= 1) return;

  XPLMGetDatavi(this->df_modeS_id, this->modeS, 0, slots);
  XPLMGetDatab(this->df_flight_id, this->flightId, 0, slots * 8);
  XPLMGetDatavf(this->df_lat, this->lat, 0, slots);
  XPLMGetDatavf(this->df_lon, this->lon, 0, slots);
  XPLMGetDatavf(this->df_ele, this->ele, 0, slots);
  XPLMGetDatavf(this->df_vx, this->vx, 0, slots);
  XPLMGetDatavf(this->df_vz, this->vz, 0, slots);
  XPLMGetDatavf(this->df_hpath, this->hpath, 0, slots);
  XPLMGetDatavf(this->df_vertical_speed, this->verticalSpeed, 0, slots);

  //keep the nearest TRAFFIC_MAX_TARGETS in range, nearest first;
  //slot 0 is our own aircraft, a slot without a Mode S id is empty
  float distances[TRAFFIC_MAX_TARGETS];
  for (int slot = 1; slot < slots; slot++)
  {
    if (!this->modeS[slot]) continue;
    float distance = latDistanceM(this->lat[0], this->lon[0], 0, this->lat[slot], this->lon[slot], 0);
    if (distance > TRAFFIC_RANGE_M) continue;

    int i = this->targetsCount < TRAFFIC_MAX_TARGETS ? this->targetsCount++ : TRAFFIC_MAX_TARGETS;
    for (; i > 0 && distances[i - 1] > distance; i--)
    {
      if (i < TRAFFIC_MAX_TARGETS)
      {
        distances[i] = distances[i - 1];
        this->targets[i] = this->targets[i - 1];
      }
    }
    if (i < TRAFFIC_MAX_TARGETS)
    {
      distances[i] = distance;
      this->targets[i] = slot;
    }
  }

  this->framesCount = 1 + this->targetsCount;
}

//==============================================================
//==============================================================
int TTraffic::encodeHeartbeat(uint8_t* buffer)
{
  TMAVLinkHeartbeat msg;
  memset(&msg, 0, sizeof(msg));
  msg.type = MAV_TYPE_ADSB;
  msg.autopilot = MAV_AUTOPILOT_INVALID;
  msg.systemStatus = MAV_STATE_ACTIVE;
  msg.mavlinkVersion = 3;
  return this->encodeFrame(buffer, MAVLINK_MSG_ID_HEARTBEAT, &msg, sizeof(msg), MAVLINK_HEARTBEAT_CRC);
}

//==============================================================
//==============================================================
int TTraffic::encodeVehicle(uint8_t* buffer, int slot)
{
  TMAVLinkADSBVehicle msg;
  memset(&msg, 0, sizeof(msg));

  msg.icao = (uint32_t)this->modeS[slot] & 0xFFFFFF;
  msg.lat = (int32_t)lround(this->lat[slot] * 1e7);
  msg.lon = (int32_t)lround(this->lon[slot] * 1e7);
  msg.altitude = (int32_t)lround(this->ele[slot] * 1000.0f);
  msg.altitudeType = ADSB_ALTITUDE_TYPE_GEOMETRIC;

  float speed = sqrtf(this->vx[slot] * this->vx[slot] + this->vz[slot] * this->vz[slot]);
  msg.horVelocity = (uint16_t)fminf(speed * 100.0f, 65535.0f);
  float verticalSpeed = this->verticalSpeed[slot] * 0.508f;  //ft/min -> cm/s
  msg.verVelocity = (int16_t)fmaxf(fminf(verticalSpeed, 32767.0f), -32768.0f);
  int heading = (int)(lround(this->hpath[slot] * 100.0f) % 36000);
  msg.heading = (uint16_t)(heading < 0 ? heading + 36000 : heading);

  memcpy(msg.callsign, this->flightId + slot * 8, 8);  //7 characters + 0

  msg.flags = ADSB_FLAGS_VALID_COORDS | ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_VELOCITY | ADSB_FLAGS_VERTICAL_VELOCITY_VALID;
  if (speed > 1.0f) msg.flags |= ADSB_FLAGS_VALID_HEADING;  //course over ground means nothing for a parked aircraft
  if (msg.callsign[0]) msg.flags |= ADSB_FLAGS_VALID_CALLSIGN;

  return this->encodeFrame(buffer, MAVLINK_MSG_ID_ADSB_VEHICLE, &msg, sizeof(msg), MAVLINK_ADSB_VEHICLE_CRC);
}

//==============================================================
//==============================================================
int TTraffic::encodeFrame(uint8_t* buffer, uint8_t messageID, const void* payload, uint8_t length, uint8_t crcExtra)
{
  buffer[0] = MAVLINK_STX_V1;
  buffer[1] = length;
  buffer[2] = this->sequence++;
  buffer[3] = TRAFFIC_MAVLINK_SYSID;
  buffer[4] = MAV_COMP_ID_ADSB;
  buffer[5] = messageID;
  memcpy(buffer + 6, payload, length);

  uint16_t crc = 0xFFFF;
  for (int i = 1; i < 6 + length; i++)
  {
    crc = crcAccumulate(buffer[i], crc);
  }
  crc = crcAccumulate(crcExtra, crc);
  buffer[6 + length] = crc & 0xFF;
  buffer[7 + length] = crc >> 8;

  return 8 + length;
}

//==============================================================
//==============================================================
void TTraffic::loadConfig(mINI::INIStructure& ini)
{
  this->sendTraffic = ini[SETTINGS_SECTION].has(SETTINGS_SEND_TRAFFIC) && (ini[SETTINGS_SECTION][SETTINGS_SEND_TRAFFIC] != "0");
}

//==============================================================
//==============================================================
void TTraffic::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_SEND_TRAFFIC] = std::to_string(this->sendTraffic ? 1 : 0);
}

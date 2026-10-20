#pragma once

#include "config.h"
#include "tcpserial.h"

#define TRAFFIC_TCAS_SLOTS  64
#define TRAFFIC_MAX_TARGETS 8

//=======================================================
//=======================================================
class TTraffic
{
public:
  bool sendTraffic = false;

  void init();
  void reset();
  void loop();

  void onSerialConfig(const uint8_t* messageBuffer, int length);
  void onSerialConfigRefused();
  void onSerialInject(bool accepted);

  void loadConfig(mINI::INIStructure& ini);
  void saveConfig(mINI::INIStructure& ini);

private:
  typedef enum
  {
    TS_FIND_PORT,   // asking INAV which serial port is set to MAVLink
    TS_PROBE,       // empty MSP2_COMMON_SERIAL_INJECT sent: does this INAV support it?
    TS_INJECT,      // traffic goes over the MSP link
    TS_SITL_TCP,    // SITL without MSP2_COMMON_SERIAL_INJECT: straight into the port's TCP socket
    TS_RETRY        // something failed: start over after retryDelay
  } TState;

  TState state = TS_FIND_PORT;
  int portIdentifier = -1;
  uint32_t lastRequestTime = 0;
  uint32_t lastCycleTime = 0;
  uint32_t lastMessageTime = 0;
  uint32_t retryDelay = 0;
  uint8_t sequence = 0;
  TCPSerial* sitlSerial = NULL;

  XPLMDataRef df_override_TCAS = NULL;
  XPLMDataRef df_num_acf = NULL;
  XPLMDataRef df_modeS_id = NULL;
  XPLMDataRef df_flight_id = NULL;
  XPLMDataRef df_lat = NULL;
  XPLMDataRef df_lon = NULL;
  XPLMDataRef df_ele = NULL;
  XPLMDataRef df_vx = NULL;
  XPLMDataRef df_vz = NULL;
  XPLMDataRef df_hpath = NULL;
  XPLMDataRef df_vertical_speed = NULL;

  // TCAS table snapshot, refreshed once per cycle. Slot 0 is our own aircraft.
  // targets[] holds the slots to send, nearest first; frame 0 of a cycle is the heartbeat.
  int targets[TRAFFIC_MAX_TARGETS];
  int targetsCount = 0;
  int framesCount = 0;
  int nextFrame = 0;
  int modeS[TRAFFIC_TCAS_SLOTS];
  char flightId[TRAFFIC_TCAS_SLOTS * 8];
  float lat[TRAFFIC_TCAS_SLOTS];
  float lon[TRAFFIC_TCAS_SLOTS];
  float ele[TRAFFIC_TCAS_SLOTS];
  float vx[TRAFFIC_TCAS_SLOTS];
  float vz[TRAFFIC_TCAS_SLOTS];
  float hpath[TRAFFIC_TCAS_SLOTS];
  float verticalSpeed[TRAFFIC_TCAS_SLOTS];

  void closeSITL();
  void retryLater();
  void readTargets();
  int encodeHeartbeat(uint8_t* buffer);
  int encodeVehicle(uint8_t* buffer, int slot);
  int encodeFrame(uint8_t* buffer, uint8_t messageID, const void* payload, uint8_t length, uint8_t crcExtra);
};

extern TTraffic g_traffic;

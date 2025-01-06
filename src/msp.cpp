#include "msp.h"
#include "util.h"

#include "serial.h"
#include "tcpserial.h"

#ifdef APL
#include <iostream>
#include <string>
#include <dirent.h>
#include <cstring>
#endif

#define MSP_DETECT_TIMEOUT_MS         300
#define MSP_COMM_TIMEOUT_MS           3000
#define MSP_COMM_DEBUG_TIMEOUT_MS     60000

#define JUMBO_FRAME_MIN_SIZE  255

#define SYM_BEGIN       '$'
#define SYM_PROTO_V1    'M'
#define SYM_PROTO_V2    'X'
#define SYM_FROM_MWC    '>'
#define SYM_TO_MWC      '<'
#define SYM_UNSUPPORTED '!'

MSP g_msp;

//======================================================
//======================================================
MSP::MSP()
{

}

//======================================================
//======================================================
MSP::~MSP()
{
  if (serial) delete serial;
}

//======================================================
//======================================================
void MSP::dispatchMessage(uint8_t expected_checksum)
{
  if (this->message_checksum == expected_checksum)
  {
    // message received, process
    this->processMessage();
  }
  else
  {
    //console.log('code: ' + this.code + ' - crc failed');
    //this.packet_error++;
    //$('span.packet-error').html(this.packet_error);
  }

  this->decoderState = DS_IDLE;
}

//======================================================
//======================================================
void MSP::decode()
{
  unsigned char data[1024];
  int len = this->serial->ReadData(data, 1024);

  if (len > 0)
  {
    this->lastUpdate = GetTicks();
  }
  else if (GetTicks() > this->lastUpdate + (IsDebuggerPresent()? MSP_COMM_DEBUG_TIMEOUT_MS : MSP_COMM_TIMEOUT_MS))
  {
    this->state = STATE_TIMEOUT;
    this->disconnect();
    return;
  }

  for (int i = 0; i < len; i++)
  {
    switch (this->decoderState)
    {
    case DS_IDLE: // sync char 1
      if (data[i] == SYM_BEGIN)
      {
        this->decoderState = DS_PROTO_IDENTIFIER;
      }
      break;

    case DS_PROTO_IDENTIFIER: // sync char 2
      switch (data[i])
      {
      case SYM_PROTO_V1:
        this->decoderState = DS_DIRECTION_V1;
        break;
      case SYM_PROTO_V2:
        this->decoderState = DS_DIRECTION_V2;
        break;
      default:
        //unknown protocol
        this->decoderState = DS_IDLE;
      }
      break;

    case DS_DIRECTION_V1: // direction (should be >)

    case DS_DIRECTION_V2:
      this->unsupported = 0;
      switch (data[i])
      {
      case SYM_FROM_MWC:
        this->message_direction = 1;
        break;
      case SYM_TO_MWC:
        this->message_direction = 0;
        break;
      case SYM_UNSUPPORTED:
        this->unsupported = 1;
        break;
      }
      this->decoderState = this->decoderState == DS_DIRECTION_V1 ? DS_PAYLOAD_LENGTH_V1 : DS_FLAG_V2;
      break;

    case DS_FLAG_V2:
      // Ignored for now
      this->decoderState = DS_CODE_V2_LOW;
      break;
    case DS_PAYLOAD_LENGTH_V1:
      this->message_length_expected = data[i];

      if (this->message_length_expected == JUMBO_FRAME_MIN_SIZE)
      {
        this->decoderState = DS_CODE_JUMBO_V1;
      }
      else
      {
        this->message_length_received = 0;
        this->decoderState = DS_CODE_V1;
      }
      break;

    case DS_PAYLOAD_LENGTH_V2_LOW:
      this->message_length_expected = data[i];
      this->decoderState = DS_PAYLOAD_LENGTH_V2_HIGH;
      break;

    case DS_PAYLOAD_LENGTH_V2_HIGH:
      this->message_length_expected |= data[i] << 8;
      this->message_length_received = 0;
      if (this->message_length_expected <= MAX_MSP_MESSAGE)
      {
        this->decoderState = this->message_length_expected > 0 ? DS_PAYLOAD_V2 : DS_CHECKSUM_V2;
      }
      else
      {
        //too large payload
        this->decoderState = DS_IDLE;
      }
      break;

    case DS_CODE_V1:
    case DS_CODE_JUMBO_V1:
      this->code = data[i];
      if (this->message_length_expected > 0)
      {
        // process payload
        if (this->decoderState == DS_CODE_JUMBO_V1)
        {
          this->decoderState = DS_PAYLOAD_LENGTH_JUMBO_LOW;
        }
        else
        {
          this->decoderState = DS_PAYLOAD_V1;
        }
      }
      else
      {
        // no payload
        this->decoderState = DS_CHECKSUM_V1;
      }
      break;

    case DS_CODE_V2_LOW:
      this->code = data[i];
      this->decoderState = DS_CODE_V2_HIGH;
      break;

    case DS_CODE_V2_HIGH:
      this->code |= data[i] << 8;
      this->decoderState = DS_PAYLOAD_LENGTH_V2_LOW;
      break;

    case DS_PAYLOAD_LENGTH_JUMBO_LOW:
      this->message_length_expected = data[i];
      this->decoderState = DS_PAYLOAD_LENGTH_JUMBO_HIGH;
      break;

    case DS_PAYLOAD_LENGTH_JUMBO_HIGH:
      this->message_length_expected |= data[i] << 8;
      this->message_length_received = 0;
      this->decoderState = DS_PAYLOAD_V1;
      break;

    case DS_PAYLOAD_V1:
    case DS_PAYLOAD_V2:
      this->message_buffer[this->message_length_received] = data[i];
      this->message_length_received++;

      if (this->message_length_received >= this->message_length_expected)
      {
        this->decoderState = this->decoderState == DS_PAYLOAD_V1 ? DS_CHECKSUM_V1 : DS_CHECKSUM_V2;
      }
      break;

    case DS_CHECKSUM_V1:
      if (this->message_length_expected >= JUMBO_FRAME_MIN_SIZE)
      {
        this->message_checksum = JUMBO_FRAME_MIN_SIZE;
      }
      else
      {
        this->message_checksum = this->message_length_expected;
      }
      this->message_checksum ^= this->code;
      if (this->message_length_expected >= JUMBO_FRAME_MIN_SIZE)
      {
        this->message_checksum ^= this->message_length_expected & 0xFF;
        this->message_checksum ^= (this->message_length_expected & 0xFF00) >> 8;
      }
      for (int ii = 0; ii < this->message_length_received; ii++)
      {
        this->message_checksum ^= this->message_buffer[ii];
      }
      this->dispatchMessage(data[i]);
      break;

    case DS_CHECKSUM_V2:
      this->message_checksum = 0;
      this->message_checksum = this->crc8_dvb_s2(this->message_checksum, 0); // flag
      this->message_checksum = this->crc8_dvb_s2(this->message_checksum, this->code & 0xFF);
      this->message_checksum = this->crc8_dvb_s2(this->message_checksum, (this->code & 0xFF00) >> 8);
      this->message_checksum = this->crc8_dvb_s2(this->message_checksum, this->message_length_expected & 0xFF);
      this->message_checksum = this->crc8_dvb_s2(this->message_checksum, (this->message_length_expected & 0xFF00) >> 8);
      for (int ii = 0; ii < this->message_length_received; ii++)
      {
        this->message_checksum = this->crc8_dvb_s2(this->message_checksum, this->message_buffer[ii]);
      }
      this->dispatchMessage(data[i]);
      break;

    default:
      break;
    }
  }
}

//======================================================
//======================================================
uint8_t MSP::crc8_dvb_s2(uint8_t crc, unsigned char a)
{
  crc ^= a;
  for (int ii = 0; ii < 8; ++ii) {
    if (crc & 0x80) {
      crc = (crc << 1) ^ 0xD5;
    }
    else {
      crc = crc << 1;
    }
  }
  return crc;
}

//======================================================
//======================================================
bool MSP::sendCommand(uint16_t messageID, void * payload, uint16_t size)
{
  if (!this->serial || !this->serial->IsConnected()) return false;

  uint8_t flag = 0;
  int msg_size = 9;
  uint8_t crc = 0;
  uint8_t tmp_buf[2];

  msg_size += (int)size;

  if (!serial->WriteData((unsigned char*)"$", 1)) return false;
  if (!serial->WriteData((unsigned char*)"X", 1)) return false;
  if (!serial->WriteData((unsigned char*)"<", 1)) return false;

  crc = MSP::crc8_dvb_s2(crc, flag);
  if (!serial->WriteData(&flag,1)) return false;

  memcpy(tmp_buf, &messageID, 2);
  crc = MSP::crc8_dvb_s2(crc, tmp_buf[0]);
  crc = MSP::crc8_dvb_s2(crc, tmp_buf[1]);
  if (!serial->WriteData(tmp_buf, 2)) return false;

  memcpy(tmp_buf, &size, 2);
  crc = MSP::crc8_dvb_s2(crc, tmp_buf[0]);
  crc = MSP::crc8_dvb_s2(crc, tmp_buf[1]);
  if (!serial->WriteData(tmp_buf, 2)) return false;

  uint8_t * payloadPtr = (uint8_t*)payload;
  for (uint8_t i = 0; i < size; ++i)
  {
    uint8_t b = *(payloadPtr++);
    crc = MSP::crc8_dvb_s2(crc, b);
    if (!serial->WriteData(&b, 1)) return false;
  }

  if (!serial->WriteData(&crc,1)) return false;

  return true;
}

//======================================================
//======================================================
bool MSP::probeNextPort()
{
  while (true)
  {
    this->portId++;
    if (this->portId == 32) return false;

    char portName[128];
#if IBM
    sprintf(portName, "\\\\.\\COM%d", portId );
#elif LIN
    sprintf(portName, "/dev/ttyACM%d", portId-1);  //start from zero on linux
#elif APL
    const char* devDir = "/dev";
    DIR* dir = opendir(devDir);

    if (dir == nullptr)
    {
      LOG("Unable to enumerate ports");
      return false;
    }

    struct dirent* entry;
    int i = this->portId;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strncmp(entry->d_name, "cu.usbmodem", 11) == 0)
      {
        i--;
        if (i == 0)
        {
          std::string portName1 = std::string(devDir) + "/" + entry->d_name;
          strcpy(portName, portName1.c_str());
          break;
        }
      }
    }

    if (entry == nullptr)
    {
      LOG("Unable to connect");
      return false;
    }

    closedir(dir);
#endif

    LOG("Probing port %s", portName);

    this->serial = new Serial();
    ((Serial*)this->serial)->OpenConnection(portName);
    if (this->serial->IsConnected())
    {
      LOG("Connected");
      if (this->sendCommand(MSP_FC_VERSION, NULL, 0))
      {
        LOG("MSP_FC_VERSION sent");
        this->state = STATE_ENUMERATE_WAIT;
        this->probeTime = GetTicks();
        this->lastUpdate = GetTicks();
        this->decoderState = DS_IDLE;
        return true;
      }
    }
    LOG("Unable to connect");
    delete this->serial;
    this->serial = NULL;
  }
}

//======================================================
//======================================================
bool MSP::connectTCP()
{
  LOG("Connecting to %s:%d", this->tcpIp, this->tcpPort);

  this->serial = new TCPSerial();
  ((TCPSerial*)this->serial)->OpenConnection(this->tcpIp, this->tcpPort);
  if (this->serial->IsConnected())
  {
    LOG("Connected");
    if (this->sendCommand(MSP_FC_VERSION, NULL, 0))
    {
      LOG("MSP_VERSION sent");
      this->state = STATE_CONNECT_TCP_WAIT;
      this->probeTime = GetTicks();
      this->lastUpdate = GetTicks();
      this->decoderState = DS_IDLE;
      return true;
    }
  }
  LOG("Unable to connect");
  delete this->serial;
  this->serial = NULL;
  return false;
}


//======================================================
//======================================================
void MSP::connect(TCBConnect cbConnect, TCBMessage cbMessage)
{
  if (state != STATE_DISCONNECTED) return;
  this->cbConnect = cbConnect;
  this->cbMessage = cbMessage;
  this->state = STATE_ENUMERATE;
  this->portId = 0;
}

//======================================================
//======================================================
void MSP::connect(TCBConnect cbConnect, TCBMessage cbMessage, const char* ip, int port)
{
  if (state != STATE_DISCONNECTED) return;
  this->cbConnect = cbConnect;
  this->cbMessage = cbMessage;
  this->state = STATE_CONNECT_TCP;

  strcpy(this->tcpIp, ip );
  this->tcpPort = port;
}

//======================================================
//======================================================
void MSP::disconnect()
{
  LOG("Disconnect");
  if (this->serial)
  {
    this->serial->flushOut();
    if (this->state == STATE_CONNECTED)
    {
      delayMS(100);  //make sure all bytes are sent. 100ms is enought to send 1kb
    }
    delete this->serial;
    this->serial = NULL;
  }

  if (this->state != STATE_DISCONNECTED)
  {
    bool timeout = this->state == STATE_TIMEOUT;
    this->state = STATE_DISCONNECTED;
    this->cbConnect( timeout? CBC_TIMEOUT_DISCONNECTED : CBC_DISCONNECTED);
  }
}

//======================================================
//======================================================
void MSP::processMessage()
{
  switch (state)
  {
  case STATE_ENUMERATE_WAIT:
  case STATE_CONNECT_TCP_WAIT:

    this->version = *((const TMSPFCVersion*)(this->message_buffer));
    LOG("Connected");
    LOG("INAV Version %d.%d.%d", this->version.major, this->version.minor, this->version.patchVersion);

    this->state = STATE_CONNECTED;
    this->cbConnect(CBC_CONNECTED);
    break;

  case STATE_CONNECTED:
    this->cbMessage(this->code, this->message_buffer, this->message_length_received);
    break;
  }
}

//======================================================
//======================================================
bool MSP::isConnected()
{
  return this->state == STATE_CONNECTED;
}

//======================================================
//======================================================
bool MSP::isConnecting()
{
  return (this->state != STATE_CONNECTED) && (this->state != STATE_DISCONNECTED);
}

//======================================================
//======================================================
void MSP::loop()
{
  switch (state)
  {
  case STATE_ENUMERATE:
    if (!this->probeNextPort())
    {
      this->state = STATE_DISCONNECTED;
      this->cbConnect(CBC_CONNECTION_FAILED);
    }
    break;

  case STATE_ENUMERATE_WAIT:
    if (GetTicks() - this->probeTime > MSP_DETECT_TIMEOUT_MS)
    {
      LOG("Probe Timeout");
      delete this->serial;
      this->serial = NULL;
      this->state = STATE_ENUMERATE;
    }
    else
    {
      this->decode();
    }
    break;

  case STATE_CONNECT_TCP_WAIT:
    if (GetTicks() - this->probeTime > MSP_DETECT_TIMEOUT_MS)
    {
      LOG("Connection Timeout");
      delete this->serial;
      this->serial = NULL;
      this->state = STATE_DISCONNECTED;
      this->cbConnect(CBC_CONNECTION_FAILED);
    }
    else
    {
      this->decode();
    }
    break;

  case STATE_CONNECTED:
    this->decode();
    break;

  case STATE_CONNECT_TCP:
    if (!this->connectTCP())
    {
      this->state = STATE_DISCONNECTED;
      this->cbConnect(CBC_CONNECTION_FAILED);
    }
    break;

  }

  if (this->serial)
  {
    this->serial->flushOut();
  }
}


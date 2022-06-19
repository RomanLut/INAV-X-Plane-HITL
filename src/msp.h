#pragma once 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "serial.h"

#define MSP_API_VERSION   1
#define MSP_SIMULATOR     256

#define MAX_MSP_MESSAGE 1024


//======================================================
//======================================================
#pragma pack(1)
struct TMSPAPIVersion
{
  uint8_t protocolVersion;
  uint8_t APIMajor;
  uint8_t APIMinor;
};
#pragma pack()

//======================================================
//======================================================
#pragma pack(1)
struct TMSPSimulatorToINAV
{
  uint8_t flags;  //1 - simulator enable

  uint8_t fix;
  uint8_t numSat;
  int32_t lat;
  int32_t lon;
  int32_t alt;
  int16_t speed;
  int16_t course;

  int16_t roll;
  int16_t pitch;
  int16_t yaw;

  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;

  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  int32_t baro;
};
#pragma pack()

//======================================================
//======================================================
#pragma pack(1)
struct TMSPSimulatorFromINAV
{
  int16_t roll;
  int16_t pitch;
  int16_t yaw;
  int16_t throttle;
};
#pragma pack()

//======================================================
//======================================================
class MSP
{
public:
	MSP();
  ~MSP();

  typedef void (*TCBConnect)(bool connected);
  typedef void(*TCBMessage)(int code, const uint8_t* messageBuffer);

  void connect(TCBConnect cbConnect, TCBMessage cbMessage);
  void disconnect();
  void loop();

  bool sendCommand(uint16_t messageID, void * payload, uint16_t size);

  bool isConnected();
  bool isConnecting();

private:
  typedef enum
  {
    STATE_DISCONNECTED,
    STATE_ENUMERATE,
    STATE_ENUMERATE_WAIT,
    STATE_CONNECTED,
  } TState;

  TState state = STATE_DISCONNECTED;

  typedef enum
  {
    DS_IDLE,
    DS_PROTO_IDENTIFIER,
    DS_DIRECTION_V1,
    DS_DIRECTION_V2,
    DS_FLAG_V2,
    DS_PAYLOAD_LENGTH_V1,
    DS_PAYLOAD_LENGTH_JUMBO_LOW,
    DS_PAYLOAD_LENGTH_JUMBO_HIGH,
    DS_PAYLOAD_LENGTH_V2_LOW,
    DS_PAYLOAD_LENGTH_V2_HIGH,
    DS_CODE_V1,
    DS_CODE_JUMBO_V1,
    DS_CODE_V2_LOW,
    DS_CODE_V2_HIGH,
    DS_PAYLOAD_V1,
    DS_PAYLOAD_V2,
    DS_CHECKSUM_V1,
    DS_CHECKSUM_V2,
  } TDecoderState;

  TDecoderState decoderState = DS_IDLE;

  uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
  bool probeNextPort();

  Serial* serial;
  int portId;
  unsigned long time;
  TCBConnect cbConnect;
  TCBMessage cbMessage;

  int unsupported;
  int message_direction;
  int message_length_expected;
  unsigned char message_buffer[MAX_MSP_MESSAGE];
  int message_length_received;
  int code;
  uint8_t message_checksum;

  void decode();
  void dispatchMessage(uint8_t expected_checksum);
  void processMessage();
};

extern MSP g_msp;

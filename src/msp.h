#pragma once 

#include "config.h"

#include "serialbase.h"

#define MSP_API_VERSION   1
#define MSP_FC_VERSION    3
#define MSP_SIMULATOR     0x201F
#define MSP_DEBUGMSG      253
#define MSP_WP_GETINFO    20
#define MSP_WP            118    //out message         get a WP, WP# is in the payload, returns (WP#, lat, lon, alt, flags) WP#0-home, WP#16-poshold


#define MAX_MSP_MESSAGE 1024

//======================================================
//======================================================
typedef enum
{
  FIF_ARMED                   = 64,
  FIF_IS_AIRPLANE             = 128,
  FIF_OSD_DISABLED            = 32,
  FIF_ANALOG_OSD_NOT_FOUND    = 16
} TFromINAVFlags;

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
struct TMSPFCVersion
{
  uint8_t major;
  uint8_t minor;
  uint8_t patchVersion;
};
#pragma pack()


//======================================================
//======================================================
#pragma pack(1)
struct TMSPWPInfo
{
  uint8_t reserved;             // Reserved for waypoint capabilities
  uint8_t maxWaypoints;         // Maximum number of waypoints supported
  uint8_t waypointsListValid;   // Is current mission valid
  uint8_t waypointsCount;       // Number of waypoints in current mission
};
#pragma pack()

//======================================================
//======================================================
#pragma pack(1)
struct TMSPWP
{
  uint8_t index;
  uint8_t action;
  int32_t lat;
  int32_t lon;
  int32_t alt; //cm
  uint16_t p1;
  uint16_t p2;
  uint16_t p3;
  uint8_t flags;
};
#pragma pack()

#define MSP_SIMULATOR_VERSION 2

//======================================================
//======================================================
typedef enum
{
  SIMU_ENABLE               = (1 << 0),
  SIMU_SIMULATE_BATTERY     = (1 << 1),
  SIMU_MUTE_BEEPER          = (1 << 2),
  SIMU_USE_SENSORS          = (1 << 3),
  SIMU_HAS_NEW_GPS_DATA     = (1 << 4),
  SIMU_EXT_BATTERY_VOLTAGE  = (1 << 5), //extend MSP_SIMULATOR format 2: battery voltage value
  SIMU_AIRSPEED             = (1 << 6),
  SIMU_EXTENDED_FLAGS       = (1 << 7), //extend MSP_SIMULATOR format 2: extra flags

  SIMU2_GPS_TIMEOUT         = (1 << 8),
  SIMU2_PITOT_FAILURE       = (1 << 9)
} TSimulatorFlags;

//======================================================
//======================================================
#pragma pack(1)
struct TMSPSimulatorToINAV
{
  uint8_t version; //MSP_SIMULATOR_VERSION

  uint8_t flags; // TSimulatorFlags
  uint8_t fix;
  uint8_t numSat;
  int32_t lat;
  int32_t lon;
  int32_t alt;
  int16_t speed;
  int16_t course;
  int16_t velNED[3];

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

  int16_t mag_x;
  int16_t mag_y;
  int16_t mag_z;

  //SIMU_EXT_BATTERY_VOLTAGE in format 2
  uint8_t vbat; //126->12.6V
  uint16_t airspeed; //cm/s

  uint8_t flags2; // TSimulatorFlags -  
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

  uint8_t debugIndex;  // | TFromINAVFlags
  int32_t debugValue;

  int16_t estimated_attitude_roll;
  int16_t estimated_attitude_pitch;
  int16_t estimated_attitude_yaw;

  union {

    struct 
    {
      //new response format
      uint8_t newFormatSignature;  // = 255

      //screen size
      uint8_t osdRows; //three high bits - format version 
      uint8_t osdCols; //two high bits - reserved

      //starting position of packet
      uint8_t osdRow; //three high bits - reserved
      uint8_t osdCol; //two high bits - reserved

      uint8_t osdRowData[400];
    } newFormat;

    struct
    {
      uint8_t osdRow; 
      uint8_t osdCol; 

      uint8_t osdRowData[400];
    } oldFormat;

  };
};
#pragma pack()

#define MSP_SIMULATOR_RESPOSE_MIN_LENGTH  (2*4 + 1 + 4 + 1)

typedef enum
{
  CBC_CONNECTED,
  CBC_CONNECTION_FAILED,
  CBC_DISCONNECTED,
  CBC_TIMEOUT_DISCONNECTED,
} TCBConnectParm;

//======================================================
//======================================================
class MSP
{
public:
	MSP();
  ~MSP();

  TMSPFCVersion version;

  typedef void (*TCBConnect)(TCBConnectParm state);
  typedef void(*TCBMessage)(int code, const uint8_t* messageBuffer, unsigned int length);

  void connect(TCBConnect cbConnect, TCBMessage cbMessage);
  void connect(TCBConnect cbConnect, TCBMessage cbMessage, const char* ip, int port);
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
    STATE_TIMEOUT,
    STATE_CONNECT_TCP,
    STATE_CONNECT_TCP_WAIT
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

  char tcpIp[MAX_PATH];
  unsigned int tcpPort;

  uint32_t lastUpdate;

  uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
  bool probeNextPort();

  SerialBase* serial;
  int portId;
  unsigned long probeTime;
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
  bool connectTCP();
};

extern MSP g_msp;

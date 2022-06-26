#pragma once

#include "config.h"
#include "simData.h"

//======================================================
//======================================================
class TStats
{
public:

  XPLMDataRef df_serialPacketsSent;
  int serialPacketsSent = 0;

  XPLMDataRef df_serialPacketsSentPerSecond;
  int serialPacketsSentPerSecond = 0;
  int serialPacketsSentLast = 0;

  XPLMDataRef df_serialBytesSent;
  int serialBytesSent = 0;

  XPLMDataRef df_serialBytesSentPerSecond;
  int serialBytesSentPerSecond = 0;
  int serialBytesSentLast = 0;

  XPLMDataRef df_serialPacketsReceived;
  int serialPacketsReceived = 0;

  XPLMDataRef df_serialPacketsReceivedPerSecond;
  int serialPacketsReceivedPerSecond = 0;
  int serialPacketsReceivedLast = 0;

  XPLMDataRef df_serialBytesReceived;
  int serialBytesReceived = 0;

  XPLMDataRef df_serialBytesReceivedPerSecond;
  int serialBytesReceivedPerSecond = 0;
  int serialBytesReceivedLast = 0;

  XPLMDataRef df_OSDUpdatesPerSecond;
  int OSDUpdates = 0;
  int OSDUpdatesLast = 0;
  int OSDUpdatesPerSecond = 0;

  XPLMDataRef df_yaw;
  int dbg_yaw = 0;
  XPLMDataRef df_pitch;
  int dbg_pitch = 0;
  XPLMDataRef df_roll;
  int dbg_roll = 0;

  XPLMDataRef df_acc_x;
  float dbg_acc_x= 0;
  XPLMDataRef df_acc_y;
  float dbg_acc_y = 0;
  XPLMDataRef df_acc_z;
  float dbg_acc_z = 0;

  XPLMDataRef df_gyro_x;
  float dbg_gyro_x = 0;
  XPLMDataRef df_gyro_y;
  float dbg_gyro_y = 0;
  XPLMDataRef df_gyro_z;
  float dbg_gyro_z = 0;

  XPLMDataRef df_debug[DEBUG_U32_COUNT];
  char debugName[DEBUG_U32_COUNT][32];
  int debug[DEBUG_U32_COUNT];

  XPLMDataRef df_cyclesPerSecond;
  int cyclesPerSecond = 0;
  int cycles = 0;
  int cyclesLast = 0;

  uint32_t lastUpdate;

  void init();
  void loop();
  void close();

private:

  XPLMDataRef registerIntDataRef( const char* pName, int *pValue);
  XPLMDataRef registerFloatDataRef(const char* pName, float *pValue);
};

extern TStats g_stats;


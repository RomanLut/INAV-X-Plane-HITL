#pragma once
#include <stdint.h>

#include "XPLMDataAccess.h"

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

  XPLMDataRef df_fDebug0;
  float fDebug0 = 0;

  XPLMDataRef df_cyclesPerSecond;
  int cyclesPerSecond = 0;
  int cycles = 0;
  int cyclesLast = 0;

  uint32_t lastUpdate;

  void init();
  void loop();
  void close();

private:

  XPLMDataRef registerIntDataRef( const char* pName, int(*GetCounterDataRefCB)(void* inRefcon), void(*SetCounterDataRefCB)(void* inRefcon, int inValue));
  XPLMDataRef registerFloatDataRef(const char* pName, float(*GetCounterDataRefCB)(void* inRefcon), void(*SetCounterDataRefCB)(void* inRefcon, float inValue));
};

extern TStats g_stats;


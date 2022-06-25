#include "stats.h"

#define MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message

TStats g_stats;

//==============================================================
//==============================================================
XPLMDataRef TStats::registerIntDataRef(const char* pName, int* pValue)
{
  XPLMDataRef res = XPLMRegisterDataAccessor(pName,
    xplmType_Int,                                  // The types we support
    1,                                             // Writable
    [](void* inRefcon) {return *((int*)inRefcon); },
    [](void* inRefcon, int inValue) { *((int*)inRefcon) = inValue; },
    // Integer accessors
    NULL, NULL,                                    // Float accessors
    NULL, NULL,                                    // Doubles accessors
    NULL, NULL,                                    // Int array accessors
    NULL, NULL,                                    // Float array accessors
    NULL, NULL,                                    // Raw data accessors
    pValue, pValue);

  XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
  if (PluginID != XPLM_NO_PLUGIN_ID)
  {
    XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)pName);
  }
  return res;
}

//==============================================================
//==============================================================
XPLMDataRef TStats::registerFloatDataRef(const char* pName, float *pValue)
{
  XPLMDataRef res = XPLMRegisterDataAccessor(pName,
    xplmType_Float,                                  // The types we support
    1,                                             // Writable
    NULL, NULL,                                    // Integer accessors
    [](void* inRefcon) {return *((float*)inRefcon); },
    [](void* inRefcon, float inValue) { *((float*)inRefcon) = inValue; },
    // Float accessors
    NULL, NULL,                                    // Doubles accessors
    NULL, NULL,                                    // Int array accessors
    NULL, NULL,                                    // Float array accessors
    NULL, NULL,                                    // Raw data accessors
    pValue, pValue);

  XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
  if (PluginID != XPLM_NO_PLUGIN_ID)
  {
    XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)pName);
  }
  return res;
}


//==============================================================
//==============================================================
void TStats::init()
{
  this->lastUpdate = GetTickCount();

  this->df_serialPacketsSent = this->registerIntDataRef("inav_hitl/serial/packetsSent", &g_stats.serialPacketsSent);
  this->df_serialPacketsSentPerSecond = this->registerIntDataRef("inav_hitl/serial/packetsSentPerSecond", &g_stats.serialPacketsSentPerSecond);
  this->df_serialBytesSent = this->registerIntDataRef("inav_hitl/serial/bytesSent", &g_stats.serialBytesSent );
  this->df_serialBytesSentPerSecond = this->registerIntDataRef("inav_hitl/serial/bytesSentPerSecond", &g_stats.serialBytesSentPerSecond );
  this->df_serialPacketsReceived = this->registerIntDataRef("inav_hitl/serial/packetsReceived", &g_stats.serialPacketsReceived);
  this->df_serialPacketsReceivedPerSecond = this->registerIntDataRef("inav_hitl/serial/packetsReceivedPerSecond", &g_stats.serialPacketsReceivedPerSecond);
  this->df_serialBytesReceived = this->registerIntDataRef("inav_hitl/serial/bytesReceived", &g_stats.serialBytesReceived);
  this->df_serialBytesReceivedPerSecond = this->registerIntDataRef("inav_hitl/serial/bytesReceivedPerSecond", &g_stats.serialBytesReceivedPerSecond);
  this->df_cyclesPerSecond = this->registerIntDataRef("inav_hitl/debug/cyclesPerSecond", &g_stats.cyclesPerSecond);
  this->df_cyclesPerSecond = this->registerIntDataRef("inav_hitl/debug/OSDUpdatesPerSecond", &g_stats.OSDUpdatesPerSecond);

  for (int i = 0; i < DEBUG_U32_COUNT; i++)
  {
    this->debug[i] = 0;
    sprintf(this->debugName[i], "inav_hitl/debug/debug[%d]", i);
    this->df_debug[i] = this->registerIntDataRef(this->debugName[i], &this->debug[i]);
  }

}

//==============================================================
//==============================================================
void TStats::loop()
{
  uint32_t delta = GetTickCount() - this->lastUpdate;
  if (delta >= 1000)
  {
    this->serialBytesSentPerSecond = this->serialBytesSent - this->serialBytesSentLast;
    this->serialBytesSentLast = this->serialBytesSent;

    this->serialPacketsSentPerSecond = this->serialPacketsSent - this->serialPacketsSentLast;
    this->serialPacketsSentLast = this->serialPacketsSent;

    this->serialBytesReceivedPerSecond = this->serialBytesReceived - this->serialBytesReceivedLast;
    this->serialBytesReceivedLast = this->serialBytesReceived;

    this->serialPacketsReceivedPerSecond = this->serialPacketsReceived - this->serialPacketsReceivedLast;
    this->serialPacketsReceivedLast = this->serialPacketsReceived;

    this->cyclesPerSecond = this->cycles - this->cyclesLast;
    this->cyclesLast = this->cycles;

    this->OSDUpdatesPerSecond = this->OSDUpdates - this->OSDUpdatesLast;
    this->OSDUpdatesLast = this->OSDUpdates;

    this->lastUpdate = GetTickCount();
  }
}

//==============================================================
//==============================================================
void TStats::close()
{
  XPLMUnregisterDataAccessor(this->df_serialPacketsSent);
  XPLMUnregisterDataAccessor(this->df_serialPacketsSentPerSecond);
  XPLMUnregisterDataAccessor(this->df_serialBytesSent);
  XPLMUnregisterDataAccessor(this->df_serialBytesSentPerSecond);
  XPLMUnregisterDataAccessor(this->df_serialPacketsReceived);
  XPLMUnregisterDataAccessor(this->df_serialPacketsReceivedPerSecond);
  XPLMUnregisterDataAccessor(this->df_serialBytesReceived);
  XPLMUnregisterDataAccessor(this->df_serialBytesReceivedPerSecond);
  XPLMUnregisterDataAccessor(this->df_OSDUpdatesPerSecond);

  for (int i = 0; i < DEBUG_U32_COUNT; i++)
  {
    XPLMUnregisterDataAccessor(this->df_debug[i]);
  }

}


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
  this->lastUpdate = GetTicks();

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

  this->df_yaw = this->registerIntDataRef("inav_hitl/inav/attitude.values.yaw", &g_stats.dbg_yaw);
  this->df_roll = this->registerIntDataRef("inav_hitl/inav/attitude.values.roll", &g_stats.dbg_roll);
  this->df_pitch = this->registerIntDataRef("inav_hitl/inav/attitude.values.pitch", &g_stats.dbg_pitch);

  this->df_acc_x = this->registerFloatDataRef("inav_hitl/inav/acc.accADCf[X]", &g_stats.dbg_acc_x);
  this->df_acc_y = this->registerFloatDataRef("inav_hitl/inav/acc.accADCf[Y]", &g_stats.dbg_acc_y);
  this->df_acc_z = this->registerFloatDataRef("inav_hitl/inav/acc.accADCf[Z]", &g_stats.dbg_acc_z);

  this->df_gyro_x = this->registerFloatDataRef("inav_hitl/inav/gyro.gyroADCf[X]", &g_stats.dbg_gyro_x);
  this->df_gyro_y = this->registerFloatDataRef("inav_hitl/inav/gyro.gyroADCf[Y]", &g_stats.dbg_gyro_y);
  this->df_gyro_z = this->registerFloatDataRef("inav_hitl/inav/gyro.gyroADCf[Z]", &g_stats.dbg_gyro_z);

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
  uint32_t delta = GetTicks() - this->lastUpdate;
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

    this->lastUpdate = GetTicks();
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

  XPLMUnregisterDataAccessor(this->df_yaw);
  XPLMUnregisterDataAccessor(this->df_roll);
  XPLMUnregisterDataAccessor(this->df_pitch);

  XPLMUnregisterDataAccessor(this->df_acc_x);
  XPLMUnregisterDataAccessor(this->df_acc_y);
  XPLMUnregisterDataAccessor(this->df_acc_z);

  XPLMUnregisterDataAccessor(this->df_gyro_x);
  XPLMUnregisterDataAccessor(this->df_gyro_y);
  XPLMUnregisterDataAccessor(this->df_gyro_z);

  for (int i = 0; i < DEBUG_U32_COUNT; i++)
  {
    XPLMUnregisterDataAccessor(this->df_debug[i]);
  }

}


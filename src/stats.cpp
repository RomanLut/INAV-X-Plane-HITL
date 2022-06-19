#include "XPLMPlugin.h"

#include "stats.h"

#define MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message

TStats g_stats;

//==============================================================
//==============================================================
XPLMDataRef TStats::registerIntDataRef(const char* pName, int(*GetCounterDataRefCB)(void* inRefcon), void(*SetCounterDataRefCB)(void* inRefcon, int inValue))
{
  XPLMDataRef res = XPLMRegisterDataAccessor(pName,
    xplmType_Int,                                  // The types we support
    1,                                             // Writable
    GetCounterDataRefCB, SetCounterDataRefCB,      // Integer accessors
    NULL, NULL,                                    // Float accessors
    NULL, NULL,                                    // Doubles accessors
    NULL, NULL,                                    // Int array accessors
    NULL, NULL,                                    // Float array accessors
    NULL, NULL,                                    // Raw data accessors
    NULL, NULL);

  XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
  if (PluginID != XPLM_NO_PLUGIN_ID)
  {
    XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)pName);
  }
  return res;
}

//==============================================================
//==============================================================
XPLMDataRef TStats::registerFloatDataRef(const char* pName, float(*GetCounterDataRefCB)(void* inRefcon), void(*SetCounterDataRefCB)(void* inRefcon, float inValue))
{
  XPLMDataRef res = XPLMRegisterDataAccessor(pName,
    xplmType_Float,                                  // The types we support
    1,                                             // Writable
    NULL, NULL,                                    // Integer accessors
    GetCounterDataRefCB, SetCounterDataRefCB,      // Float accessors
    NULL, NULL,                                    // Doubles accessors
    NULL, NULL,                                    // Int array accessors
    NULL, NULL,                                    // Float array accessors
    NULL, NULL,                                    // Raw data accessors
    NULL, NULL);

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

  this->df_serialPacketsSent = this->registerIntDataRef("inav_hitl/serial/packetsSent",
    [] (void* inRefcon) {return g_stats.serialPacketsSent; },
    [](void* inRefcon, int inValue) { g_stats.serialPacketsSent = inValue;  }
  );

  this->df_serialPacketsSentPerSecond = this->registerIntDataRef("inav_hitl/serial/packetsSentPerSecond",
    [](void* inRefcon) {return g_stats.serialPacketsSentPerSecond; },
    [](void* inRefcon, int inValue) { g_stats.serialPacketsSentPerSecond = inValue;  }
  );

  this->df_serialBytesSent = this->registerIntDataRef("inav_hitl/serial/bytesSent",
    [](void* inRefcon) {return g_stats.serialBytesSent; },
    [](void* inRefcon, int inValue) { g_stats.serialBytesSent = inValue;  }
    );

  this->df_serialBytesSentPerSecond = this->registerIntDataRef("inav_hitl/serial/bytesSentPerSecond",
    [](void* inRefcon) {return g_stats.serialBytesSentPerSecond; },
    [](void* inRefcon, int inValue) { g_stats.serialBytesSentPerSecond = inValue;  }
  );

  this->df_serialPacketsReceived = this->registerIntDataRef("inav_hitl/serial/packetsReceived",
    [](void* inRefcon) {return g_stats.serialPacketsReceived; },
    [](void* inRefcon, int inValue) { g_stats.serialPacketsReceived = inValue;  }
  );

  this->df_serialPacketsReceivedPerSecond = this->registerIntDataRef("inav_hitl/serial/packetsReceivedPerSecond",
    [](void* inRefcon) {return g_stats.serialPacketsReceivedPerSecond; },
    [](void* inRefcon, int inValue) { g_stats.serialPacketsReceivedPerSecond = inValue;  }
  );

  this->df_serialBytesReceived = this->registerIntDataRef("inav_hitl/serial/bytesReceived",
    [](void* inRefcon) {return g_stats.serialBytesReceived; },
    [](void* inRefcon, int inValue) { g_stats.serialBytesReceived = inValue;  }
    );

  this->df_serialBytesReceivedPerSecond = this->registerIntDataRef("inav_hitl/serial/bytesReceivedPerSecond",
    [](void* inRefcon) {return g_stats.serialBytesReceivedPerSecond; },
    [](void* inRefcon, int inValue) { g_stats.serialBytesReceivedPerSecond = inValue;  }
  );

  this->df_fDebug0 = this->registerFloatDataRef("inav_hitl/debug/fDebug0",
    [](void* inRefcon) {return g_stats.fDebug0; },
    [](void* inRefcon, float inValue) { g_stats.fDebug0 = inValue;  }
  );
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
  XPLMUnregisterDataAccessor(this->df_fDebug0);
}


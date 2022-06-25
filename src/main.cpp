#include "config.h"

#define MSP_TIMEOUT_MS  1000u
#define MSP_PERIOD_MS  10u

#include "menu.h"
#include "simData.h"
#include "msp.h"
#include "stats.h"
#include "util.h"
#include "osd.h"

#include "config.h"

uint32_t lastUpdateTime;
bool wait;

XPLMFlightLoopID loopId;

//==============================================================
//==============================================================
void cbConnect(TCBConnectParm state)
{
  g_menu._cbConnect(state);
  g_osd.cbConnect(state);
  lastUpdateTime = GetTickCount();
  wait = false;
}

//==============================================================
//==============================================================
void cbMessage(int code, const uint8_t* messageBuffer, int length)
{
  if (code == MSP_SIMULATOR)
  {
    g_simData.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);
    g_simData.sendToXPlane();

    g_osd.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);

    wait = false;
  }
  else if (code == MSP_DEBUGMSG)
  {
    LOG("INAV: %s", (const char*)messageBuffer);
  }

}

//==============================================================
//==============================================================
// this flightloop callback will be called every frame to update the targets
float floop_cb(float elapsed1, float elapsed2, int ctr, void* refcon)
{
  g_msp.loop();

  if (g_msp.isConnected())
  {
    if ((GetTickCount() - lastUpdateTime) > (wait ? MSP_TIMEOUT_MS : MSP_PERIOD_MS ))
    {
      g_simData.updateFromXPlane();
      g_simData.sendToINAV();
      lastUpdateTime = GetTickCount();
      wait = true;
    }
  }

  g_stats.cycles++;
  g_stats.loop();
  return -1;
}

//==============================================================
//==============================================================
PLUGIN_API int XPluginStart(
	char *		outName,
	char *		outSig,
	char *		outDesc)
{
  LOG("Plugin start\n");

	strcpy(outName, "INAV HITL");
	strcpy(outSig, "https://github.com/iNavFlight");
	strcpy(outDesc, "INAV Hardware in the loop");

  g_osd.init();

	return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void	XPluginStop(void)
{
  LOG("Plugin stop\n");

  g_osd.destroy();
}

//==============================================================
//==============================================================

PLUGIN_API int XPluginEnable(void) 
{ 
  LOG("Plugin enable\n");

  g_stats.init();
  g_simData.init(); //initialize before memu
  g_simData.updateFromXPlane();

  g_menu.createMenu();

  XPLMCreateFlightLoop_t params;
  params.structSize = sizeof(XPLMCreateFlightLoop_t);
  params.callbackFunc = &floop_cb;
  params.phase = xplm_FlightLoop_Phase_AfterFlightModel;
  params.refcon = NULL;
  loopId = XPLMCreateFlightLoop(&params);
  XPLMScheduleFlightLoop(loopId, -1, true);

  playSound("assets\\ready_to_connect.wav");

  return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginDisable(void)
{
  LOG("Plugin disable\n");

  g_stats.close();
  g_msp.disconnect();
  g_menu.destroyMenu();
  XPLMDestroyFlightLoop(loopId);
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) 
{ 
}


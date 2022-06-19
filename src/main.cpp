#include "XPLMMenus.h"

#include "XPLMProcessing.h"
//#include "XPLMDataAccess.h"
//#include "XPLMUtilities.h"
//#include "XPLMPlugin.h"

#include <string.h>
#if IBM
#include <windows.h>
#endif

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

#define MSP_TIMEOUT_MS  1000u
#define MSP_PERIOD_MS  10u

#include "menu.h"
#include "simData.h"
#include "msp.h"
#include "stats.h"
#include "playsound.h"

uint32_t lastUpdateTime;
bool wait;

//==============================================================
//==============================================================
void cbConnect( bool connected)
{
  g_menu._cbConnect(connected);
  lastUpdateTime = GetTickCount();
  wait = false;
}

//==============================================================
//==============================================================
void cbMessage(int code, const uint8_t* messageBuffer)
{
  if (code == MSP_SIMULATOR)
  {
    g_simData.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);
    g_simData.sendToXPlane();
    wait = false;
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
	strcpy(outName, "INAV HITL");
	strcpy(outSig, "https://github.com/iNavFlight");
	strcpy(outDesc, "INAV Hardware in the loop");

	return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void	XPluginStop(void)
{
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginDisable(void) 
{
  g_stats.close();
  g_msp.disconnect();
  g_menu.destroyMenu();
  XPLMUnregisterFlightLoopCallback(floop_cb, NULL);
}

//==============================================================
//==============================================================

PLUGIN_API int XPluginEnable(void) 
{ 
  g_stats.init();
  g_simData.init(); //initialize before memu
  g_simData.updateFromXPlane();

  g_menu.createMenu();

  XPLMRegisterFlightLoopCallback(floop_cb, 1, NULL);

  playSound("assets\\ready_to_connect.wav");

  return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) 
{ 
}


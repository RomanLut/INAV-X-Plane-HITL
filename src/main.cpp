#include "config.h"

#define MSP_TIMEOUT_MS  1000u
#define MSP_PERIOD_MS  10u

#include "menu.h"
#include "simData.h"
#include "msp.h"
#include "stats.h"
#include "util.h"
#include "osd.h"
#include "graph.h"

#include "config.h"

uint32_t lastUpdateTime;
bool wait;
bool firstRender = true;

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
    if (length < MSP_SIMULATOR_RESPOSE_MIN_LENGTH)
    {
      g_menu.actionDisconnect();                          
      playSound("assets\\unsupported_firmware.wav");
      LOG("Unsupported firmware version, MSP_SIMULATOR response length: %d", length);
      g_osd.showMsg("UNSUPPORTED FIRMWARE");
    }
    else
    {
      g_simData.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);

      if (!g_simData.isAirplane)
      {
        g_menu.actionDisconnect();
        playSound("assets\\unsupported.wav");
        LOG("Unsupported aircraft type");
        g_osd.showMsg("UNSUPPORTED AIRCRAFT TYPE");
      }
      else
      {
        g_simData.sendToXPlane();

        g_osd.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);

        wait = false;
      }
    }
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
      //wait = true; do not wait for answer, send next state after 10us passed
    }
  }

  g_stats.cycles++;
  g_stats.loop();
  return -1;
}

//==============================================================
//==============================================================
int	drawCallback(
  XPLMDrawingPhase     inPhase,
  int                  inIsBefore,
  void *               inRefcon)
{
  if (firstRender)
  {
    setView();
    g_osd.setHome();
    firstRender = false;
  }

  g_osd.drawCallback();
  g_graph.drawCallback();
  return 1;
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
	strcpy(outSig, "https://github.com/RomanLut/INAV-X-Plane-HITL");
	strcpy(outDesc, "INAV Hardware In The Loop");

  g_osd.init();

  XPLMRegisterDrawCallback(&drawCallback, xplm_Phase_LastCockpit, 0, NULL);

	return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void	XPluginStop(void)
{
  LOG("Plugin stop\n");

  XPLMUnregisterDrawCallback(&drawCallback, xplm_Phase_FirstCockpit, 0, NULL);

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

  if (g_msp.isConnected())
  {
    g_simData.disconnect();
    g_msp.disconnect();
  }
  g_menu.destroyMenu();

  XPLMDestroyFlightLoop(loopId);
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) 
{
  if (inMsg == XPLM_MSG_AIRPORT_LOADED)
  {
    firstRender = true;
  }
}



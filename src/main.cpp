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
#include "sound.h"
#include "map.h"

uint32_t lastUpdateTime;
bool should_wait;
bool firstRender = true;

XPLMFlightLoopID loopId;

mINI::INIStructure ini;

//==============================================================
//==============================================================
void cbConnect(TCBConnectParm state)
{
  g_menu._cbConnect(state);
  g_osd.cbConnect(state);
  g_simData.setBateryEmulation(g_simData.batEmulation); //rechange battery
  lastUpdateTime = GetTicks();
  should_wait = false;
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
      TMSPSimulatorFromINAV msg;
      memset(&msg, 0, sizeof(msg));
      if (length > sizeof(msg)) return;
      memcpy(&msg, messageBuffer, length);
      g_simData.updateFromINAV(&msg);

      if (!g_simData.isAirplane)
      {
        g_menu.actionDisconnect();
        playSound("assets\\unsupported.wav");
        LOG("Unsupported aircraft type");
        g_osd.showMsg("UNSUPPORTED AIRCRAFT TYPE");
      }
      else
      {
        if (g_simData.isOSDDisabled)
        {
          g_osd.showMsg("OSD IS DISABLED:\n\nCONFIURATION\n->OTHER FEATURES\n->OSD");
        }
        else if (g_simData.isSupportedOSDNotFound)
        {
          g_osd.showMsg("SUPPORTED OSD TYPE\nNOT FOUND OR DISABLED\n\nCHECK REQUIREMENTS\nON PROJECT PAGE");
        }

        g_simData.sendToXPlane();

        g_osd.updateFromINAV((const TMSPSimulatorFromINAV*)messageBuffer);

        should_wait = false;
      }
    }
  }
  else if (code == MSP_DEBUGMSG)
  {
    LOG("INAV: %s\n", (const char*)messageBuffer);
  }
  else if (code == MSP_WP_GETINFO)
  {
    g_map.onWPInfo((const TMSPWPInfo*)messageBuffer);
  }
  else if (code == MSP_WP)
  {
    g_map.onWP((const TMSPWP*)messageBuffer);
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
    if ((GetTicks() - lastUpdateTime) > (should_wait ? MSP_TIMEOUT_MS : MSP_PERIOD_MS ))
    {
      g_simData.updateFromXPlane();
      g_simData.sendToINAV();
      lastUpdateTime = GetTicks();
      //should_wait = true; do not wait for answer, send next state after 10us passed
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
  LOG("Plugin start");

	strcpy(outName, "INAV HITL");
	strcpy(outSig, "https://github.com/RomanLut/INAV-X-Plane-HITL");
	strcpy(outDesc, "INAV Hardware In The Loop");

  XPLMRegisterDrawCallback(&drawCallback, xplm_Phase_Window/*xplm_Phase_LastCockpit*/, 0, NULL);

	return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void	XPluginStop(void)
{
  LOG("Plugin stop");

  XPLMUnregisterDrawCallback(&drawCallback, xplm_Phase_FirstCockpit, 0, NULL);

  g_sound.destroy();
  g_map.destroy();

}

//==============================================================
//==============================================================
bool buildIniFileName( char* iniFileName )
{
  XPLMGetPrefsPath(iniFileName);
  char* p = XPLMExtractFileAndPath(iniFileName);
  if (!p) return false;
  strncat(iniFileName, XPLMGetDirectorySeparator(), MAX_PATH);
  strcat(iniFileName, "inavhitl.ini");
  return true;
}

//==============================================================
//==============================================================
bool loadIniFile()
{
  char iniFileName[MAX_PATH];
  if (!buildIniFileName(iniFileName))
  {
    g_menu.updateAll();
    return false;
  }

  mINI::INIFile iniFile(iniFileName);
  iniFile.read(ini);

  g_simData.loadConfig(ini);
  g_osd.loadConfig(ini);
  g_graph.loadConfig(ini);
  g_map.loadConfig(ini);
  g_menu.loadConfig(ini);

  g_menu.updateAll();

  return true;
}


//==============================================================
//==============================================================
bool saveIniFile()
{
  g_simData.saveConfig(ini);
  g_osd.saveConfig(ini);
  g_graph.saveConfig(ini);
  g_map.saveConfig(ini);
  g_menu.saveConfig(ini);

  char iniFileName[MAX_PATH];
  if (!buildIniFileName(iniFileName)) return false;

  mINI::INIFile iniFile(iniFileName);
  iniFile.generate(ini);
  return true;
}

//==============================================================
//==============================================================
PLUGIN_API int XPluginEnable(void)
{
  LOG("Plugin enable");

  g_osd.init();
  g_stats.init();
  g_simData.init(); //initialize before memu
  g_simData.updateFromXPlane();

  g_map.init();

  g_menu.createMenu();

  XPLMCreateFlightLoop_t params;
  params.structSize = sizeof(XPLMCreateFlightLoop_t);
  params.callbackFunc = &floop_cb;
  params.phase = xplm_FlightLoop_Phase_AfterFlightModel;
  params.refcon = NULL;
  loopId = XPLMCreateFlightLoop(&params);
  XPLMScheduleFlightLoop(loopId, -1, true);

  g_sound.init();

  playSound("assets\\ready_to_connect.wav");

  loadIniFile();

  return 1;
}

//==============================================================
//==============================================================
PLUGIN_API void XPluginDisable(void)
{
  LOG("Plugin disable");

  g_osd.destroy();
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



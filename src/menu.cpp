#include "menu.h"

#include "msp.h"
#include "util.h"
#include "simData.h"
#include "osd.h"

TMenu g_menu;

extern void cbConnect(TCBConnectParm state);
extern void cbMessage(int code, const uint8_t* messageBuffer, int length);

//==============================================================
//==============================================================
void TMenu::_cbConnect(TCBConnectParm state)
{
  if (state == CBC_CONNECTED)
  {
    XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Disconnect from Flight Controller", 0);
    playSound("assets\\connected.wav");
  }
  else 
  {
    XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Connect to Flight Controller", 0);
    if (state == CBC_CONNECTION_FAILED)
    {
      playSound("assets\\connection_failed.wav");
    }
    else if (state == CBC_TIMEOUT_DISCONNECTED)
    {
      playSound("assets\\connection_lost.wav");
    }
  }
}

//==============================================================
//==============================================================
void TMenu::updateGPSMenu()
{
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_0_id, g_simData.gps_numSat == 0 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_12_id, g_simData.gps_numSat == 12 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateOSDMenu()
{
  XPLMCheckMenuItem(this->osd_menu_id, this->osd_none_id, g_osd.osd_type == OSD_NONE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_menu_id, this->osd_auto_id, g_osd.osd_type == OSD_AUTO ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_menu_id, this->osd_pal_id, g_osd.osd_type == OSD_PAL ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_menu_id, this->osd_ntsc_id, g_osd.osd_type == OSD_NTSC ? xplm_Menu_Checked : xplm_Menu_Unchecked);

  XPLMCheckMenuItem(this->osd_menu_id, this->osd_nearest_id, g_osd.smoothed == false ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_menu_id, this->osd_linear_id, g_osd.smoothed == true ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateBatteryMenu()
{
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_default_id, g_simData.emulateBattery == false ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_emulate_id, g_simData.emulateBattery == true ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateBeeperMenu()
{
  XPLMCheckMenuItem(this->beeper_menu_id, this->beeper_default_id, g_simData.muteBeeper == false ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->beeper_menu_id, this->beeper_mute_id, g_simData.muteBeeper == true ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::static_menu_handler(void * in_menu_ref, void * in_item_ref)
{
  g_menu.menu_handler(in_menu_ref, in_item_ref);
}

//==============================================================
//==============================================================
void TMenu::menu_handler(void * in_menu_ref, void * in_item_ref)
{
  /*
	if (!strcmp((const char *)in_item_ref, "Menu Item 1"))
	{
		XPLMCommandOnce(XPLMFindCommand("sim/operation/toggle_settings_window"));
	}
	else if (!strcmp((const char *)in_item_ref, "Menu Item 2"))
	{
		XPLMCommandOnce(XPLMFindCommand("sim/operation/toggle_key_shortcuts_window"));
	}
  */

  if (!strcmp((const char *)in_item_ref, "connect_disconnect"))
  {
    if (!g_msp.isConnecting())
    {
      if (g_msp.isConnected())
      {
        XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Connect to Flight Controller", 0);
        g_simData.disconnect();
        g_msp.disconnect();
      }
      else
      {
        g_msp.connect(&cbConnect, &cbMessage);
      }
    }
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_0"))
  {
    g_simData.gps_numSat = GPS_NO_FIX;
    g_simData.gps_fix = 0;
    g_simData.gps_spoofing = 0;
    this->updateGPSMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_12"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_spoofing = 0;
    this->updateGPSMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_none"))
  {
    g_osd.osd_type = OSD_NONE;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_auto"))
  {
    g_osd.osd_type = OSD_AUTO;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_pal"))
  {
    g_osd.osd_type = OSD_PAL;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_ntsc"))
  {
    g_osd.osd_type = OSD_NTSC;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_nearest"))
  {
    g_osd.smoothed = false;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "osd_linear"))
  {
    g_osd.smoothed = true;
    this->updateOSDMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "battery_default"))
  {
    g_simData.emulateBattery = false;
    this->updateBatteryMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "battery_emulate"))
  {
    g_simData.emulateBattery = true;
    this->updateBatteryMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "beeper_default"))
  {
    g_simData.muteBeeper = false;
    this->updateBeeperMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "beeper_mute"))
  {
    g_simData.muteBeeper = true;
    this->updateBeeperMenu();
  }

}

//==============================================================
//==============================================================
void TMenu::createMenu()
{
	this->menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "INAV HITL", 0, 0);

	this->menu_id = XPLMCreateMenu("INAV HITL", XPLMFindPluginsMenu(), this->menu_container_idx, static_menu_handler, NULL);
	this->connect_id = XPLMAppendMenuItem(this->menu_id, "Link", (void *)"Menu Item 1", 1);
	this->connect_menu_id = XPLMCreateMenu("Link", this->menu_id, this->connect_id, static_menu_handler, NULL);
	this->connect_disconnect_id = XPLMAppendMenuItem(this->connect_menu_id, "Connect to Flight Controller", (void *)"connect_disconnect", 1);

	XPLMAppendMenuSeparator(this->menu_id);

	this->gps_fix_id = XPLMAppendMenuItem(this->menu_id, "GPS Fix", (void *)"gps_fix", 1);
	this->gps_fix_menu_id = XPLMCreateMenu("GPS Fix", this->menu_id, this->gps_fix_id, static_menu_handler, NULL);
	this->gps_fix_0_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "0 satellites (No fix)", (void *)"gps_fix_0", 1);
	this->gps_fix_12_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "12 satellites (3D fix)", (void *)"gps_fix_12", 1);

  this->osd_id = XPLMAppendMenuItem(this->menu_id, "OSD", (void *)"OSD", 1);
  this->osd_menu_id = XPLMCreateMenu("OSD", this->menu_id, this->osd_id, static_menu_handler, NULL);
  this->osd_none_id = XPLMAppendMenuItem(this->osd_menu_id, "None", (void *)"osd_none", 1);
  this->osd_auto_id = XPLMAppendMenuItem(this->osd_menu_id, "Auto", (void *)"osd_auto", 1);
  this->osd_pal_id = XPLMAppendMenuItem(this->osd_menu_id, "PAL", (void *)"osd_pal", 1);
  this->osd_ntsc_id = XPLMAppendMenuItem(this->osd_menu_id, "NTSC", (void *)"osd_ntsc", 1);

  this->battery_id = XPLMAppendMenuItem(this->menu_id, "Battery", (void *)"Battery", 1);
  this->battery_menu_id = XPLMCreateMenu("Battery", this->menu_id, this->battery_id, static_menu_handler, NULL);
  this->battery_default_id = XPLMAppendMenuItem(this->battery_menu_id, "Default", (void *)"battery_default", 1);
  this->battery_emulate_id = XPLMAppendMenuItem(this->battery_menu_id, "Emulate 3s", (void *)"battery_emulate", 1);

  this->beeper_id = XPLMAppendMenuItem(this->menu_id, "Beeper", (void *)"Beeper", 1);
  this->beeper_menu_id = XPLMCreateMenu("Battery", this->menu_id, this->beeper_id, static_menu_handler, NULL);
  this->beeper_default_id = XPLMAppendMenuItem(this->beeper_menu_id, "Default", (void *)"beeper_default", 1);
  this->beeper_mute_id = XPLMAppendMenuItem(this->beeper_menu_id, "Mute", (void *)"beeper_mute", 1);

  XPLMAppendMenuSeparator(this->osd_menu_id);

  this->osd_nearest_id = XPLMAppendMenuItem(this->osd_menu_id, "Smoothing: Nearest", (void *)"osd_nearest", 1);
  this->osd_linear_id = XPLMAppendMenuItem(this->osd_menu_id, "Smoothing: Linear", (void *)"osd_linear", 1);

/*                               
	this->gps_spoofing_1_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Freeze", (void *)"gps_spoofing_1", 1);
	this->gps_spoofing_2_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Linear 1m/s", (void *)"gps_spoofing_2", 1);
	this->gps_spoofing_3_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Random jump 100m", (void *)"gps_spoofing_3", 1);
	this->gps_spoofing_4_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Random jump 100m + satellites count", (void *)"gps_spoofing_4", 1);
	this->gps_spoofing_5_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Circle 500m 1m/s", (void *)"gps_spoofing_5", 1);
*/
  this->updateGPSMenu();
  this->updateOSDMenu();
  this->updateBatteryMenu();
  this->updateBeeperMenu();
}

//==============================================================
//==============================================================
void TMenu::destroyMenu()
{
  XPLMDestroyMenu(g_menu.menu_id);
}


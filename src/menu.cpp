#include "menu.h"

#include "msp.h"
#include "playSound.h"
#include "simData.h"

TMenu g_menu;

extern void cbConnect(bool connected);
extern void cbMessage(int code, const uint8_t* messageBuffer);

//==============================================================
//==============================================================
void TMenu::_cbConnect(bool connected)
{
  if (connected)
  {
    XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Disconnect from Flight Controller", 0);
    playSound("assets\\connected.wav");
  }
  else
  {
    //todo: messagebox fail
    XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Connect to Flight Controller", 0);
    playSound("assets\\connection_failed.wav");
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
    g_simData.gps_numSat = 0;
    g_simData.gps_fix = 0;
    g_simData.gps_spoofing = 0;
    this->updateGPSMenu();
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_12"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = 2;
    g_simData.gps_spoofing = 0;
    this->updateGPSMenu();
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
/*                               
	this->gps_spoofing_1_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Freeze", (void *)"gps_spoofing_1", 1);
	this->gps_spoofing_2_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Linear 1m/s", (void *)"gps_spoofing_2", 1);
	this->gps_spoofing_3_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Random jump 100m", (void *)"gps_spoofing_3", 1);
	this->gps_spoofing_4_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Random jump 100m + satellites count", (void *)"gps_spoofing_4", 1);
	this->gps_spoofing_5_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "Spoofing: Circle 500m 1m/s", (void *)"gps_spoofing_5", 1);
*/
  this->updateGPSMenu();
}

//==============================================================
//==============================================================
void TMenu::destroyMenu()
{
  XPLMDestroyMenu(g_menu.menu_id);
}


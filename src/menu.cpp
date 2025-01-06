#include "menu.h"

#include "msp.h"
#include "util.h"
#include "simData.h"
#include "osd.h"
#include "graph.h"
#include "map.h"
#include "ipinputwidget.h"

TMenu g_menu;

extern void cbConnect(TCBConnectParm state);
extern void cbMessage(int code, const uint8_t* messageBuffer, int length);

//==============================================================
//==============================================================
void TMenu::setDisconnectedMenuState()
{
  XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, "Connect to Flight Controller", 0);
  for (int i = 0; i < 8; i++)
  {
    XPLMEnableMenuItem(this->connect_sitl_menu_id, this->connect_tcp_id[i], true);
  }
  XPLMEnableMenuItem(this->connect_menu_id, this->connect_sitl_id, true);
}


//==============================================================
//==============================================================
void TMenu::_cbConnect(TCBConnectParm state)
{
  if (state == CBC_CONNECTED)                             
  {
    XPLMSetMenuItemName(this->connect_menu_id, this->connect_disconnect_id, !this->isSITLConnection ? "Disconnect from Flight Controller" : "Disconnect from SITL", 0);
    for (int i = 0; i < 8; i++)
    {
      XPLMEnableMenuItem(this->connect_sitl_menu_id, this->connect_tcp_id[i], false);
    }
    XPLMEnableMenuItem(this->connect_menu_id, this->connect_sitl_id, false);
    playSound("assets\\connected.wav");
    this->setFeatures();
  }
  else 
  {
    this->setDisconnectedMenuState();
    if (state == CBC_CONNECTION_FAILED)
    {                                                  
      playSound("assets\\connection_failed.wav");
      LOG("cbConnect(): Connection failed");
    }
    else if (state == CBC_TIMEOUT_DISCONNECTED)
    {
      playSound("assets\\connection_lost.wav");
      LOG("cbConnect(): Connection lost");
    }
  }
}

//==============================================================
//==============================================================
void TMenu::updateGPSMenu()
{
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_0_id, (g_simData.gps_numSat == 0) && !g_simData.gps_timeout? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_3_id, (g_simData.gps_numSat == 3) && !g_simData.gps_timeout ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_5_id, (g_simData.gps_numSat == 5) && !g_simData.gps_timeout && !g_simData.gps_glitch ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_fix_12_id, (g_simData.gps_numSat == 12) && !g_simData.gps_timeout && !g_simData.gps_glitch? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_timeout_id, g_simData.gps_timeout ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_freeze_id, g_simData.gps_glitch == GPS_GLITCH_FREEZE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_offset_id, g_simData.gps_glitch == GPS_GLITCH_OFFSET ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_linear_id, g_simData.gps_glitch == GPS_GLITCH_LINEAR ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_circle_id, g_simData.gps_glitch == GPS_GLITCH_CIRCLE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_altitude_id, g_simData.gps_glitch == GPS_GLITCH_ALTITUDE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_circle_altitude_id, g_simData.gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->gps_fix_menu_id, this->gps_circle_altitude5_id, g_simData.gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE_5 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateMagMenu()
{
  XPLMCheckMenuItem(this->mag_menu_id, this->mag_normal_id, (g_simData.simulate_mag_failure == false) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->mag_menu_id, this->mag_failure_id, (g_simData.simulate_mag_failure == true) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateAttitudeMenu()
{
  XPLMCheckMenuItem(this->attitude_menu_id, this->attitude_force_id, g_simData.attitude_use_sensors == false ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->attitude_menu_id, this->attitude_sensors_id, g_simData.attitude_use_sensors == true ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateOSDMenu()
{
  XPLMCheckMenuItem(this->osd_visibility_menu_id, this->osd_visibility_on_id, g_osd.visible ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_visibility_menu_id, this->osd_visibility_off_id, !g_osd.visible ? xplm_Menu_Checked : xplm_Menu_Unchecked);

  XPLMCheckMenuItem(this->osd_rows_menu_id, this->osd_rows_auto_id, g_osd.osd_type == OSD_AUTO ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_rows_menu_id, this->osd_rows_pal_id, g_osd.osd_type == OSD_PAL ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_rows_menu_id, this->osd_rows_ntsc_id, g_osd.osd_type == OSD_NTSC ? xplm_Menu_Checked : xplm_Menu_Unchecked);

  XPLMCheckMenuItem(this->osd_filtering_menu_id, this->osd_filtering_nearest_id, g_osd.smoothed == false ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->osd_filtering_menu_id, this->osd_filtering_linear_id, g_osd.smoothed == true ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateBatteryMenu()
{
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_none_id, g_simData.batEmulation == BATTERY_NONE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_infinite_id, g_simData.batEmulation == BATTERY_INFINITE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_discharged_id, g_simData.batEmulation == BATTERY_DISCHAGED ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_3min_id, g_simData.batEmulation == BATTERY_3MIN ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_10min_id, g_simData.batEmulation == BATTERY_10MIN ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->battery_menu_id, this->battery_30min_id, g_simData.batEmulation == BATTERY_30MIN ? xplm_Menu_Checked : xplm_Menu_Unchecked);
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
void TMenu::updatePitotMenu()
{
  XPLMCheckMenuItem(this->pitot_menu_id, this->pitot_none_id, (g_simData.simulatePitot == false) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->pitot_menu_id, this->pitot_simulate_id, (g_simData.simulatePitot == true) && ((g_simData.simulatePitotFailureHW || g_simData.simulatePitotFailure60 )== false) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->pitot_menu_id, this->pitot_failure_hw_id, (g_simData.simulatePitot == true) && (g_simData.simulatePitotFailureHW == true) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->pitot_menu_id, this->pitot_failure_60_id, (g_simData.simulatePitot == true) && (g_simData.simulatePitotFailure60 == true) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateGraphMenu()
{
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_none_id, g_graph.getGraphType() == GRAPH_NONE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_updates_id, g_graph.getGraphType() == GRAPH_UPDATES ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_attitude_estimation_id, g_graph.getGraphType() == GRAPH_ATTITUDE_ESTIMATION ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_attitude_output_id, g_graph.getGraphType() == GRAPH_ATTITUDE_OUTPUT ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_acc_id, g_graph.getGraphType() == GRAPH_ACC ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_gyro_id, g_graph.getGraphType() == GRAPH_GYRO ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_debug_altitude_id, g_graph.getGraphType() == GRAPH_DEBUG_ALTITUDE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->graph_menu_id, this->graph_debug_custom_id, g_graph.getGraphType() == GRAPH_DEBUG_CUSTOM ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateMapMenu()
{
  XPLMCheckMenuItem(this->map_menu_id, this->map_none_id, g_map.getMarkingType() == MMT_NONE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->map_menu_id, this->map_lat_lon_osd_id, g_map.getMarkingType() == MMT_LAT_LON_OSD ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->map_menu_id, this->map_debug_0_1_id, g_map.getMarkingType() == MMT_DEBUG_0_1 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateNoiseMenu()
{
  XPLMCheckMenuItem(this->noise_menu_id, this->noise_none_id, g_osd.videoLink == VS_NONE ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->noise_menu_id, this->noise_2KM_id, g_osd.videoLink == VS_2KM ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->noise_menu_id, this->noise_10KM_id, g_osd.videoLink == VS_10KM ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  XPLMCheckMenuItem(this->noise_menu_id, this->noise_50KM_id, g_osd.videoLink == VS_50KM ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

//==============================================================
//==============================================================
void TMenu::updateSITLMenu()
{

  for (int i = 0; i < 8; i++)
  {
    strcpy(this->connect_tcp_name[i], "UART1 (");
    this->connect_tcp_name[i][4] += i;
    strcat(this->connect_tcp_name[i], this->SITLIP.c_str());
    strcat(this->connect_tcp_name[i], ":5760)");
    this->connect_tcp_name[i][strlen(this->connect_tcp_name[i])-2] += i;
    XPLMSetMenuItemName(this->connect_sitl_menu_id, this->connect_tcp_id[i], this->connect_tcp_name[i], 0);
  }
}

//==============================================================
//==============================================================
void TMenu::static_menu_handler(void * in_menu_ref, void * in_item_ref)
{
  g_menu.menu_handler(in_menu_ref, in_item_ref);
}

//==============================================================
//==============================================================
void TMenu::actionDisconnect()
{
  this->setDisconnectedMenuState();
  g_simData.disconnect();
  g_msp.disconnect();
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
        this->actionDisconnect();
      }
      else
      {
        this->isSITLConnection = false;
        g_msp.connect(&cbConnect, &cbMessage);
      }
    }
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_0"))
  {
    g_simData.gps_numSat = GPS_NO_FIX;
    g_simData.gps_fix = 0;
    g_simData.gps_glitch = 0;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_3"))
  {
    g_simData.gps_numSat = 3;
    g_simData.gps_fix = GPS_NO_FIX;
    g_simData.gps_glitch = 0;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_5"))
  {
    g_simData.gps_numSat = 5;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = 0;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_fix_12"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = 0;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_timeout"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = 0;
    g_simData.gps_timeout = true;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_freeze"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_FREEZE;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_offset"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_OFFSET;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_linear"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_LINEAR;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_circle"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_CIRCLE;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_altitude"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_ALTITUDE;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_circle_altitude"))
  {
    g_simData.gps_numSat = 12;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_CIRCLE_ALTITUDE;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "gps_glitch_circle_altitude_5"))
  {
    g_simData.gps_numSat = 5;
    g_simData.gps_fix = GPS_FIX_3D;
    g_simData.gps_glitch = GPS_GLITCH_CIRCLE_ALTITUDE;
    g_simData.gps_timeout = false;
  }
  else if (!strcmp((const char *)in_item_ref, "mag_normal"))
  {
    g_simData.simulate_mag_failure = false;
  }
  else if (!strcmp((const char *)in_item_ref, "mag_failure"))
  {
    g_simData.simulate_mag_failure = true;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_enabled"))
  {
    g_osd.visible = true;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_disabled"))
  {
    g_osd.visible = false;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_auto"))
  {
    g_osd.osd_type = OSD_AUTO;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_pal"))
  {
    g_osd.osd_type = OSD_PAL;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_ntsc"))
  {
    g_osd.osd_type = OSD_NTSC;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_nearest"))
  {
    g_osd.smoothed = false;
  }
  else if (!strcmp((const char *)in_item_ref, "osd_linear"))
  {
    g_osd.smoothed = true;
  }
  else if (!strcmp((const char *)in_item_ref, "battery_none"))
  {
    g_simData.setBateryEmulation(BATTERY_NONE);
  }
  else if (!strcmp((const char *)in_item_ref, "battery_infinite"))
  {
    g_simData.setBateryEmulation(BATTERY_INFINITE);
  }
  else if (!strcmp((const char *)in_item_ref, "battery_discharged"))
  {
    g_simData.setBateryEmulation(BATTERY_DISCHAGED);
  }
  else if (!strcmp((const char *)in_item_ref, "battery_3min"))
  {
    g_simData.setBateryEmulation(BATTERY_3MIN);
  }
  else if (!strcmp((const char *)in_item_ref, "battery_10min"))
  {
    g_simData.setBateryEmulation(BATTERY_10MIN);
  }
  else if (!strcmp((const char *)in_item_ref, "battery_30min"))
  {
    g_simData.setBateryEmulation(BATTERY_30MIN);
  }
  else if (!strcmp((const char *)in_item_ref, "beeper_default"))
  {
    g_simData.muteBeeper = false;
  }
  else if (!strcmp((const char *)in_item_ref, "beeper_mute"))
  {
    g_simData.muteBeeper = true;
  }
  else if (!strcmp((const char *)in_item_ref, "pitot_none"))
  {
    g_simData.simulatePitot = false;
    g_simData.simulatePitotFailureHW = false;
    g_simData.simulatePitotFailure60 = false;
  }
  else if (!strcmp((const char *)in_item_ref, "pitot_simulate"))
  {
    g_simData.simulatePitot = true;
    g_simData.simulatePitotFailureHW = false;
    g_simData.simulatePitotFailure60 = false;
  }
  else if (!strcmp((const char *)in_item_ref, "pitot_simulate_failure_hw"))
  {
    g_simData.simulatePitot = true;
    g_simData.simulatePitotFailureHW = true;
    g_simData.simulatePitotFailure60 = false;
  }
  else if (!strcmp((const char *)in_item_ref, "pitot_simulate_failure_60"))
  {
  g_simData.simulatePitot = true;
  g_simData.simulatePitotFailureHW = false;
  g_simData.simulatePitotFailure60 = true;
  }
  else if (!strcmp((const char *)in_item_ref, "attitude_force"))
  {
    g_simData.attitude_use_sensors = false;
  }
  else if (!strcmp((const char *)in_item_ref, "attitude_sensors"))
  {
    g_simData.attitude_use_sensors = true;
  }
  else if (!strcmp((const char *)in_item_ref, "graph_none"))
  {
    g_graph.setGraphType(GRAPH_NONE);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_updates"))
  {
    g_graph.setGraphType(GRAPH_UPDATES);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_attitude_estimation"))
  {
    g_graph.setGraphType(GRAPH_ATTITUDE_ESTIMATION);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_attitude_output"))
  {
    g_graph.setGraphType(GRAPH_ATTITUDE_OUTPUT);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_accelerometer"))
  {
    g_graph.setGraphType(GRAPH_ACC);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_gyroscope"))
  {
    g_graph.setGraphType(GRAPH_GYRO);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_debug_altitude"))
  {
    g_graph.setGraphType(GRAPH_DEBUG_ALTITUDE);
  }
  else if (!strcmp((const char *)in_item_ref, "graph_debug_custom"))
  {
    g_graph.setGraphType(GRAPH_DEBUG_CUSTOM);
  }
  else if (!strcmp((const char *)in_item_ref, "map_none"))
  {
    g_map.setMarkingType(MMT_NONE);
  }
  else if (!strcmp((const char *)in_item_ref, "map_lat_lon_osd"))
  {
    g_map.setMarkingType(MMT_LAT_LON_OSD);
  }
  else if (!strcmp((const char *)in_item_ref, "map_debug_0_1"))
  {
    g_map.setMarkingType(MMT_DEBUG_0_1);
  }
  else if (!strcmp((const char *)in_item_ref, "map_clear_tracks"))
  {
    g_map.clearTracks();
  }
  else if (!strcmp((const char *)in_item_ref, "map_download_waypoints"))
  {
    g_map.startDownloadWaypoints();
  }
  else if (!strcmp((const char *)in_item_ref, "map_teleport"))
  {
    g_map.teleport();
  }
  else if (!strcmp((const char *)in_item_ref, "noise_none"))
  {
    g_osd.videoLink = VS_NONE;
  }
  else if (!strcmp((const char *)in_item_ref, "noise_2km"))
  {
    g_osd.videoLink = VS_2KM;
  }
  else if (!strcmp((const char *)in_item_ref, "noise_10km"))
  {
    g_osd.videoLink = VS_10KM;
  }
  else if (!strcmp((const char *)in_item_ref, "noise_50km"))
  {
    g_osd.videoLink = VS_50KM;
  }
  else if (!strcmp((const char*)in_item_ref, "sitl_ip"))
  {
    g_IPInputWidget.setValue(this->SITLIP);
    g_IPInputWidget.show();
  }
  else if (!strcmp((const char*)in_item_ref, "autolaunch_kick"))
  {
    g_simData.autolaunch_kickStart = GetTicks();
  }
  else
  {
    int index = g_osd.getFontIndexByName((const char *)in_item_ref);
    if (index >= 0)
    {
      g_osd.setActiveFontByIndex(index);
    }

    for (int i = 0; i < 8; i++)
    {
      if (!strcmp((const char *)in_item_ref, this->connect_tcp_tag[i]))
      {
        if (!g_msp.isConnecting())
        {
          if (g_msp.isConnected())
          {
            this->actionDisconnect();
          }
          else
          {
            this->isSITLConnection = true;
            g_msp.connect(&cbConnect, &cbMessage, this->SITLIP.c_str(), 5760 + i);
          }
        }
      }
    }
  }

  this->updateAll();
  saveIniFile();
}

//==============================================================
//==============================================================
void TMenu::createMenu()
{
  this->menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "INAV HITL/SITL", 0, 0);

  this->menu_id = XPLMCreateMenu("INAV HITL", XPLMFindPluginsMenu(), this->menu_container_idx, static_menu_handler, NULL);
  this->connect_id = XPLMAppendMenuItem(this->menu_id, "Link", (void *)"Menu Item 1", 1);
  this->connect_menu_id = XPLMCreateMenu("Link", this->menu_id, this->connect_id, static_menu_handler, NULL);
  this->connect_disconnect_id = XPLMAppendMenuItem(this->connect_menu_id, "Connect to Flight Controller", (void *)"connect_disconnect", 1);

  this->connect_sitl_id = XPLMAppendMenuItem(this->connect_menu_id, "Connect to SITL", NULL, 1);
  this->connect_sitl_menu_id = XPLMCreateMenu("Connect to SITL", this->connect_menu_id, this->connect_sitl_id, static_menu_handler, NULL);

  for (int i = 0; i < 8; i++)
  {
    strcpy(this->connect_tcp_name[i], "UARTN");
    strcpy(this->connect_tcp_tag[i], "connect_sitl_0");
    this->connect_tcp_tag[i][13] += i;
    this->connect_tcp_id[i] = XPLMAppendMenuItem(this->connect_sitl_menu_id, this->connect_tcp_name[i], (void *)(this->connect_tcp_tag[i]), 1);
  }
  this->updateSITLMenu();

  XPLMAppendMenuSeparator(this->menu_id);

  this->gps_fix_id = XPLMAppendMenuItem(this->menu_id, "GPS Fix", (void *)"gps_fix", 1);
  this->gps_fix_menu_id = XPLMCreateMenu("GPS Fix", this->menu_id, this->gps_fix_id, static_menu_handler, NULL);              
  this->gps_fix_0_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "0 satellites (No fix)", (void *)"gps_fix_0", 1);
  this->gps_fix_3_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "3 satellites (No fix)", (void *)"gps_fix_3", 1);
  this->gps_fix_5_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "5 satellites (3D fix)", (void *)"gps_fix_5", 1);
  this->gps_fix_12_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "12 satellites (3D fix)", (void *)"gps_fix_12", 1);
  this->gps_timeout_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[HW Failure] Sensor timeout", (void *)"gps_timeout", 1);
  this->gps_freeze_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Freeze position", (void *)"gps_glitch_freeze", 1);
  this->gps_offset_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Apply 5km offset", (void *)"gps_glitch_offset", 1);
  this->gps_linear_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Apply linear shift 10m/s", (void *)"gps_glitch_linear", 1);
  this->gps_circle_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Circle 1km", (void *)"gps_glitch_circle", 1);
  this->gps_altitude_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Altitude up sawtooth", (void *)"gps_glitch_altitude", 1);
  this->gps_circle_altitude_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Circle + altitude up sawtooth", (void *)"gps_glitch_circle_altitude", 1);
  this->gps_circle_altitude5_id = XPLMAppendMenuItem(this->gps_fix_menu_id, "[GPS Glitch] Circle + altitude up sawtooth, 5 satellites ", (void *)"gps_glitch_circle_altitude_5", 1);

  this->mag_id = XPLMAppendMenuItem(this->menu_id, "Compass", (void *)"Compass", 1);
  this->mag_menu_id = XPLMCreateMenu("Compass", this->menu_id, this->mag_id, static_menu_handler, NULL);
  this->mag_normal_id = XPLMAppendMenuItem(this->mag_menu_id, "Normal", (void *)"mag_normal", 1);
  this->mag_failure_id = XPLMAppendMenuItem(this->mag_menu_id, "HW Failure", (void *)"mag_failure", 1);

  this->attitude_id = XPLMAppendMenuItem(this->menu_id, "Attitude", (void *)"attitude", 1);
  this->attitude_menu_id = XPLMCreateMenu("Attitude", this->menu_id, this->attitude_id, static_menu_handler, NULL);
  this->attitude_force_id = XPLMAppendMenuItem(this->attitude_menu_id, "Copy from X-Plane", (void *)"attitude_force", 1);
  this->attitude_sensors_id = XPLMAppendMenuItem(this->attitude_menu_id, "Estimate from sensors", (void *)"attitude_sensors", 1);

  this->osd_id = XPLMAppendMenuItem(this->menu_id, "OSD", (void *)"OSD", 1);
  this->osd_menu_id = XPLMCreateMenu("OSD", this->menu_id, this->osd_id, static_menu_handler, NULL);

  this->osd_visibility_id = XPLMAppendMenuItem(this->osd_menu_id, "Visibility", (void *)"OSD VISIBILITY", 1);
  this->osd_visibility_menu_id = XPLMCreateMenu("Visibility", this->osd_menu_id, this->osd_visibility_id, static_menu_handler, NULL);
  this->osd_visibility_on_id = XPLMAppendMenuItem(this->osd_visibility_menu_id, "OSD Enabled", (void *)"osd_enabled", 1);
  this->osd_visibility_off_id = XPLMAppendMenuItem(this->osd_visibility_menu_id, "OSD Disabled", (void *)"osd_disabled", 1);

  this->osd_fonts_id = XPLMAppendMenuItem(this->osd_menu_id, "Font", (void *)"Font", 1);
  this->osd_fonts_menu_id = XPLMCreateMenu("Font", this->osd_menu_id, this->osd_fonts_id, static_menu_handler, NULL);

  this->osd_analog_fonts_id = XPLMAppendMenuItem(this->osd_fonts_menu_id, "Analog OSD", (void *)"Analog OSD", 1);
  this->osd_analog_fonts_menu_id = XPLMCreateMenu("Analog OSD", this->osd_fonts_menu_id, this->osd_analog_fonts_id, static_menu_handler, NULL);

  this->osd_digital_fonts_id = XPLMAppendMenuItem(this->osd_fonts_menu_id, "Digital OSD", (void *)"Digital OSD", 1);
  this->osd_digital_fonts_menu_id = XPLMCreateMenu("Digital OSD", this->osd_fonts_menu_id, this->osd_digital_fonts_id, static_menu_handler, NULL);

  this->osd_rows_id = XPLMAppendMenuItem(this->osd_menu_id, "Analog OSD Rows", (void *)"OSD ROWS", 1);
  this->osd_rows_menu_id = XPLMCreateMenu("Analog OSD Rows", this->osd_menu_id, this->osd_rows_id, static_menu_handler, NULL);
  this->osd_rows_auto_id = XPLMAppendMenuItem(this->osd_rows_menu_id, "Auto", (void *)"osd_auto", 1);
  this->osd_rows_pal_id = XPLMAppendMenuItem(this->osd_rows_menu_id, "PAL", (void *)"osd_pal", 1);
  this->osd_rows_ntsc_id = XPLMAppendMenuItem(this->osd_rows_menu_id, "NTSC", (void *)"osd_ntsc", 1);

  this->osd_filtering_id = XPLMAppendMenuItem(this->osd_menu_id, "Font Rendering", (void *)"FontRendering", 1);
  this->osd_filtering_menu_id = XPLMCreateMenu("Font Rendering", this->osd_menu_id, this->osd_filtering_id, static_menu_handler, NULL);
  this->osd_filtering_nearest_id = XPLMAppendMenuItem(this->osd_filtering_menu_id, "Smoothing: Nearest", (void *)"osd_nearest", 1);
  this->osd_filtering_linear_id = XPLMAppendMenuItem(this->osd_filtering_menu_id, "Smoothing: Linear", (void *)"osd_linear", 1);


  this->battery_id = XPLMAppendMenuItem(this->menu_id, "Battery", (void *)"Battery", 1);
  this->battery_menu_id = XPLMCreateMenu("Battery", this->menu_id, this->battery_id, static_menu_handler, NULL);
  this->battery_none_id = XPLMAppendMenuItem(this->battery_menu_id, "Do not simulate", (void *)"battery_none", 1);
  this->battery_infinite_id = XPLMAppendMenuItem(this->battery_menu_id, "Infinite 3s", (void *)"battery_infinite", 1);
  this->battery_discharged_id = XPLMAppendMenuItem(this->battery_menu_id, "Discharged 3s", (void *)"battery_discharged", 1);
  this->battery_3min_id = XPLMAppendMenuItem(this->battery_menu_id, "3 minutes 3s", (void *)"battery_3min", 1);
  this->battery_10min_id = XPLMAppendMenuItem(this->battery_menu_id, "10 minutes 3s", (void *)"battery_10min", 1);
  this->battery_30min_id = XPLMAppendMenuItem(this->battery_menu_id, "30 minutes 3s", (void *)"battery_30min", 1);

  this->beeper_id = XPLMAppendMenuItem(this->menu_id, "Beeper", (void *)"Beeper", 1);
  this->beeper_menu_id = XPLMCreateMenu("Battery", this->menu_id, this->beeper_id, static_menu_handler, NULL);
  this->beeper_default_id = XPLMAppendMenuItem(this->beeper_menu_id, "Default", (void *)"beeper_default", 1);
  this->beeper_mute_id = XPLMAppendMenuItem(this->beeper_menu_id, "Mute", (void *)"beeper_mute", 1);

  this->pitot_id = XPLMAppendMenuItem(this->menu_id, "Pitot", (void *)"Pitot", 1);
  this->pitot_menu_id = XPLMCreateMenu("Pitot", this->menu_id, this->pitot_id, static_menu_handler, NULL);
  this->pitot_none_id = XPLMAppendMenuItem(this->pitot_menu_id, "Do not simulate", (void *)"pitot_none", 1);
  this->pitot_simulate_id = XPLMAppendMenuItem(this->pitot_menu_id, "Simulate", (void *)"pitot_simulate", 1);
  this->pitot_failure_hw_id = XPLMAppendMenuItem(this->pitot_menu_id, "Simulate failure", (void *)"pitot_simulate_failure_hw", 1);
  this->pitot_failure_60_id = XPLMAppendMenuItem(this->pitot_menu_id, "Simulate failure: stuck at 60 km/h", (void *)"pitot_simulate_failure_60", 1);

  //XPLMAppendMenuSeparator(this->menu_id);

  this->noise_id = XPLMAppendMenuItem(this->menu_id, "Analog Video", (void *)"Analog Video", 1);
  this->noise_menu_id = XPLMCreateMenu("Video", this->menu_id, this->noise_id, static_menu_handler, NULL);
  this->noise_none_id = XPLMAppendMenuItem(this->noise_menu_id, "No simulation", (void *)"noise_none", 1);
  this->noise_2KM_id = XPLMAppendMenuItem(this->noise_menu_id, "Link up to 2km", (void *)"noise_2km", 1);
  this->noise_10KM_id = XPLMAppendMenuItem(this->noise_menu_id, "Link up to 10km", (void *)"noise_10km", 1);
  this->noise_50KM_id = XPLMAppendMenuItem(this->noise_menu_id, "Link up to 50km", (void *)"noise_50km", 1);

  XPLMAppendMenuSeparator(this->menu_id);

  this->map_id = XPLMAppendMenuItem(this->menu_id, "Map", (void *)"Map", 1);
  this->map_menu_id = XPLMCreateMenu("Map", this->menu_id, this->map_id, static_menu_handler, NULL);
  this->map_none_id = XPLMAppendMenuItem(this->map_menu_id, "None", (void *)"map_none", 1);
  this->map_lat_lon_osd_id = XPLMAppendMenuItem(this->map_menu_id, "Latitude/Longitude from OSD", (void *)"map_lat_lon_osd", 1);
  this->map_debug_0_1_id = XPLMAppendMenuItem(this->map_menu_id, "debug[0]/debug[1] as Latitude/Longitude", (void *)"map_debug_0_1", 1);
  XPLMAppendMenuSeparator(this->map_menu_id);
  this->map_clear_tracks = XPLMAppendMenuItem(this->map_menu_id, "Clear tracks", (void *)"map_clear_tracks", 1);
  this->map_download_waypoints = XPLMAppendMenuItem(this->map_menu_id, "Download waypoints from FC", (void *)"map_download_waypoints", 1);
  this->map_teleport = XPLMAppendMenuItem(this->map_menu_id, "Teleport to location (from clipboard)", (void *)"map_teleport", 1);

  this->graph_id = XPLMAppendMenuItem(this->menu_id, "Graph", (void *)"Graph", 1);
  this->graph_menu_id = XPLMCreateMenu("Graph", this->menu_id, this->graph_id, static_menu_handler, NULL);
  this->graph_none_id = XPLMAppendMenuItem(this->graph_menu_id, "None", (void *)"graph_none", 1);
  this->graph_updates_id = XPLMAppendMenuItem(this->graph_menu_id, "Update period, MS", (void *)"graph_updates", 1);
  this->graph_attitude_estimation_id = XPLMAppendMenuItem(this->graph_menu_id, "Attitude estimation", (void *)"graph_attitude_estimation", 1);
  this->graph_attitude_output_id = XPLMAppendMenuItem(this->graph_menu_id, "Attitude, output", (void *)"graph_attitude_output", 1);
  this->graph_acc_id = XPLMAppendMenuItem(this->graph_menu_id, "Accelerometer", (void *)"graph_accelerometer", 1);
  this->graph_gyro_id = XPLMAppendMenuItem(this->graph_menu_id, "Gyroscope", (void *)"graph_gyroscope", 1);
  this->graph_debug_altitude_id = XPLMAppendMenuItem(this->graph_menu_id, "debug_mode = altitude", (void *)"graph_debug_altitude", 1);
  this->graph_debug_custom_id = XPLMAppendMenuItem(this->graph_menu_id, "debug[8] array", (void *)"graph_debug_custom", 1);

  this->action_id = XPLMAppendMenuItem(this->menu_id, "Action", (void *)"Action", 1);
  this->action_menu_id = XPLMCreateMenu("Action", this->menu_id, this->action_id, static_menu_handler, NULL);
  this->autolaunch_id = XPLMAppendMenuItem(this->action_menu_id, "Autolaunch kick 4g", (void *)"autolaunch_kick", 1);

  g_osd.addFontsToMenu();

  g_IPInputWidget.create(300, 100, 300, 105, "IP Address", "IP Address");
  g_IPInputWidget.registerValueChangedCb(TMenu::ipChangedHandlerStatic);

  XPLMAppendMenuSeparator(this->connect_sitl_menu_id);
  XPLMAppendMenuItem(this->connect_sitl_menu_id, "Edit SITL IP address", (void*)"sitl_ip", 1);

  this->updateAll();
}

//==============================================================
//==============================================================
void TMenu::updateAll()
{
  this->updateGPSMenu();
  this->updateMagMenu();
  this->updateOSDMenu();
  this->updateBatteryMenu();
  this->updateBeeperMenu();
  this->updatePitotMenu();
  this->updateAttitudeMenu();
  this->updateNoiseMenu();
  this->updateGraphMenu();
  this->updateMapMenu();
  this->updateSITLMenu();
}

//==============================================================
//==============================================================
void TMenu::destroyMenu()
{
  XPLMDestroyMenu(g_menu.menu_id);
}

//==============================================================
//==============================================================
void TMenu::addFontEntry(bool analog, const char* fontName)
{
  if (analog)
  {
    this->analogFontMenuItems.push_back(XPLMAppendMenuItem(this->osd_analog_fonts_menu_id, fontName, (void *)fontName, 1));
  }
  else
  {
    this->digitalFontMenuItems.push_back(XPLMAppendMenuItem( this->osd_digital_fonts_menu_id, fontName, (void *)fontName, 1));
  }
}

//==============================================================
//==============================================================
void TMenu::updateFontsMenu(int activeAnalogFontIndex, int activeDigitalFontIndex)
{
  for (int i = 0; i < this->analogFontMenuItems.size(); i++)
  {
    XPLMCheckMenuItem(this->osd_analog_fonts_menu_id, this->analogFontMenuItems[i], i == activeAnalogFontIndex ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  }

  for (int i = 0; i < this->digitalFontMenuItems.size(); i++)
  {
    XPLMCheckMenuItem(this->osd_digital_fonts_menu_id, this->digitalFontMenuItems[i], i == (activeDigitalFontIndex - analogFontMenuItems.size()) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  }
}

//==============================================================
//==============================================================
void TMenu::setFeatures()
{
  bool b;

  //GPS timeout 
  b = g_msp.version.major >= 7;
  if (!b)
  {
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(this->gps_fix_menu_id, this->gps_timeout_id, &check);
    if (check == xplm_Menu_Checked)
    {
      g_simData.gps_timeout = false;
      updateGPSMenu();
    }
  }
  XPLMEnableMenuItem(this->gps_fix_menu_id, this->gps_timeout_id, b);

  //mag failure
  b = g_msp.version.major >= 7;
  if (!b)
  {
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(this->mag_menu_id, this->mag_failure_id, &check);
    if (check == xplm_Menu_Checked)
    {
      g_simData.simulate_mag_failure = false;
      this->updateMagMenu();
    }
  }
  XPLMEnableMenuItem(this->mag_menu_id, this->mag_failure_id, b);

  //pitot failure
  b = g_msp.version.major >= 7;
  if (!b)
  {
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(this->pitot_menu_id, this->pitot_failure_hw_id, &check);
    if (check == xplm_Menu_Checked)
    {
      g_simData.simulatePitotFailureHW = false;
      this->updatePitotMenu();
    }
  }
  XPLMEnableMenuItem(this->pitot_menu_id, pitot_failure_hw_id, b);
}

//==============================================================
//==============================================================
void TMenu::ipChangedHandlerStatic(std::string ip)
{
  if (!validateIpAddress(ip))
  {
    XPLMSpeakString("Invalid IP Address");
    LOG("Invalid IP Address");
    g_IPInputWidget.setValue(g_menu.SITLIP);
    return;
  }
  g_menu.SITLIP = ip;
  g_menu.updateSITLMenu();
  saveIniFile();
}

//==============================================================
//==============================================================
void TMenu::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_SITL_IP] = this->SITLIP;
}

//==============================================================
//==============================================================
void TMenu::loadConfig(mINI::INIStructure& ini)
{
  if (ini[SETTINGS_SECTION].has(SETTINGS_SITL_IP))
  {
    this->SITLIP = ini[SETTINGS_SECTION][SETTINGS_SITL_IP];
  }
  else
  {
    this->SITLIP = "127.0.0.1";
  }
}

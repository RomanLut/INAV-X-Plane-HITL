#pragma once

#include "config.h"

#include "msp.h"

//=======================================================
//=======================================================
class TMenu
{
public:
	int menu_container_idx; // The index of our menu item in the Plugins menu

	XPLMMenuID menu_id;
	int connect_id;
	int gps_fix_id;
	int gps_spoofing_id;

	XPLMMenuID connect_menu_id;
	int connect_disconnect_id;

	XPLMMenuID gps_fix_menu_id;
	int gps_fix_0_id;
	int gps_fix_12_id;

  XPLMMenuID osd_menu_id;
  int osd_id;
  int osd_none_id;
  int osd_auto_id;
  int osd_pal_id;
  int osd_ntsc_id;

  int osd_nearest_id;
  int osd_linear_id;

  XPLMMenuID attitude_menu_id;
  int attitude_id;
  int attitude_force_id;
  int attitude_sensors_id;

  XPLMMenuID battery_menu_id;
  int battery_id;
  int battery_default_id;
  int battery_emulate_id;

  XPLMMenuID beeper_menu_id;
  int beeper_id;
  int beeper_default_id;
  int beeper_mute_id;

  XPLMMenuID graph_menu_id;
  int graph_id;
  int graph_none_id;
  int graph_updates_id;
  int graph_attitude_estimation_id;
  int graph_attitude_output_id;
  int graph_acc_id;
  int graph_gyro_id;
  int graph_debug_altitude_id;
  int graph_debug_custom_id;

  XPLMMenuID noise_menu_id;
  int noise_id;
  int noise_none_id;
  int noise_2KM_id;
  int noise_10KM_id;
  int noise_50KM_id;

	int gps_spoofing_1_id;
	int gps_spoofing_2_id;
	int gps_spoofing_3_id;
	int gps_spoofing_4_id;
	int gps_spoofing_5_id;

	static void static_menu_handler(void * in_menu_ref, void * in_item_ref);
  void menu_handler(void * in_menu_ref, void * in_item_ref);
  void createMenu();
  void destroyMenu();
  void _cbConnect(TCBConnectParm state);
  void updateGPSMenu();
  void updateOSDMenu();
  void updateBatteryMenu();
  void updateBeeperMenu();
  void updateAttitudeMenu();
  void updateGraphMenu();
  void updateNoiseMenu();

  void actionDisconnect();
};

extern TMenu g_menu;

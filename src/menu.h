#pragma once

#include "config.h"

#include "msp.h"

//=======================================================
//=======================================================
class TMenu
{
private:
  std::vector<int> analogFontMenuItems;
  std::vector<int> digitalFontMenuItems;

  void setDisconnectedMenuState();
  static void ipChangedHandlerStatic(std::string ip);

public:
	int menu_container_idx; // The index of our menu item in the Plugins menu

	XPLMMenuID menu_id;
	int connect_id;
  int connect_tcp_id[8];
  int gps_fix_id;
  bool isSITLConnection;

  std::string SITLIP = "127.0.0.1";

  XPLMMenuID connect_sitl_menu_id;
  int connect_sitl_id;

  char connect_tcp_name[8][64];
  char connect_tcp_tag[8][32];

	XPLMMenuID connect_menu_id;
	int connect_disconnect_id;

	XPLMMenuID gps_fix_menu_id;
	int gps_fix_0_id;
  int gps_fix_3_id;
  int gps_fix_5_id;
  int gps_fix_12_id;
  int gps_timeout_id;
  int gps_freeze_id;
  int gps_offset_id;
  int gps_linear_id;
  int gps_circle_id;
  int gps_altitude_id;
  int gps_circle_altitude_id;
  int gps_circle_altitude5_id;

  XPLMMenuID osd_menu_id;
  int osd_id;

  XPLMMenuID osd_visibility_menu_id;
  int osd_visibility_id;
  int osd_visibility_on_id;
  int osd_visibility_off_id;

  XPLMMenuID osd_fonts_menu_id;
  int osd_fonts_id;
  XPLMMenuID osd_analog_fonts_menu_id;
  int osd_analog_fonts_id;
  XPLMMenuID osd_digital_fonts_menu_id;
  int osd_digital_fonts_id;


  XPLMMenuID osd_rows_menu_id;
  int osd_rows_id;
  int osd_rows_auto_id;
  int osd_rows_pal_id;
  int osd_rows_ntsc_id;

  XPLMMenuID osd_filtering_menu_id;
  int osd_filtering_id;
  int osd_filtering_nearest_id;
  int osd_filtering_linear_id;

  XPLMMenuID attitude_menu_id;
  int attitude_id;
  int attitude_force_id;
  int attitude_sensors_id;

  XPLMMenuID battery_menu_id;
  int battery_id;
  int battery_none_id;
  int battery_infinite_id;
  int battery_discharged_id;
  int battery_3min_id;
  int battery_10min_id;
  int battery_30min_id;

  XPLMMenuID beeper_menu_id;
  int beeper_id;
  int beeper_default_id;
  int beeper_mute_id;

  XPLMMenuID mag_menu_id;
  int mag_id;
  int mag_normal_id;
  int mag_failure_id;

  XPLMMenuID pitot_menu_id;
  int pitot_id;
  int pitot_none_id;
  int pitot_simulate_id;
  int pitot_failure_hw_id;
  int pitot_failure_60_id;

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

  XPLMMenuID map_menu_id;
  int map_id;
  int map_none_id;
  int map_lat_lon_osd_id;
  int map_debug_0_1_id;
  int map_download_waypoints;
  int map_clear_tracks;
  int map_teleport;

  XPLMMenuID action_menu_id;
  int action_id;
  int autolaunch_id;

  XPLMMenuID noise_menu_id;
  int noise_id;
  int noise_none_id;
  int noise_2KM_id;
  int noise_10KM_id;
  int noise_50KM_id;

	static void static_menu_handler(void * in_menu_ref, void * in_item_ref);
  void menu_handler(void * in_menu_ref, void * in_item_ref);
  void createMenu();
  void destroyMenu();
  void _cbConnect(TCBConnectParm state);
  void updateGPSMenu();
  void updateMagMenu();
  void updateOSDMenu();
  void updateBatteryMenu();
  void updateBeeperMenu();
  void updatePitotMenu();
  void updateAttitudeMenu();
  void updateGraphMenu();
  void updateMapMenu();
  void updateNoiseMenu();
  void updateSITLMenu();
  void updateAll();

  void updateFontsMenu(int activeAnalogFontIndex, int activeDigitalFontIndex);

  void actionDisconnect();

  void addFontEntry( bool analog, const char* fontName);

  void setFeatures();

  void saveConfig(mINI::INIStructure& ini);
  void loadConfig(mINI::INIStructure& ini);
};

extern TMenu g_menu;

#pragma once

#include "XPLMMenus.h"

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
	int gps_spoofing_1_id;
	int gps_spoofing_2_id;
	int gps_spoofing_3_id;
	int gps_spoofing_4_id;
	int gps_spoofing_5_id;

	static void static_menu_handler(void * in_menu_ref, void * in_item_ref);
  void menu_handler(void * in_menu_ref, void * in_item_ref);
  void createMenu();
  void destroyMenu();
  void _cbConnect(bool connected);
  void updateGPSMenu();
};

extern TMenu g_menu;

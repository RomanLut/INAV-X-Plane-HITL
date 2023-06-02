#pragma once

#include "config.h"

extern void destroyTexture(int textureId);
extern std::vector<std::string> getAvaiableFonts(void); 

extern void setView();

extern void buildAssetFilename(char pName[MAX_PATH], const char* pFileName);
extern void playSound(const char* pFileName);

extern void LOG(const char* fmt, ...);

extern float latDistanceM(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2);

extern void disableBrakes();
extern void setView();

extern float clampf(float value, float minValue, float maxValue);
extern int16_t clampToInt16(float value);

extern void delayMS(uint32_t valueMS);

extern void getClipboardText(char str[1024]);

extern bool saveIniFile();

#ifdef LIN
extern bool IsDebuggerPresent();
extern uint32_t GetTickCount();
#endif

#define SETTINGS_SECTION                "settings"
#define SETTINGS_GPS_NUMSAT             "gps_numSat"
#define SETTINGS_GPS_TIMEOUT            "gps_timeout"
#define SETTINGS_MAG_FAILURE            "mag_failure"
#define SETTINGS_ATTITUDE_USE_SENSORS   "attitude_use_sensors"
#define SETTINGS_OSD_TYPE               "osd_type"
#define SETTINGS_OSD_SMOOTHED           "osd_smoothed"
#define SETTINGS_BATTERY_EMULATION      "battery_emulation"
#define SETTINGS_MUTE_BEEPER            "mute_beeper"
#define SETTINGS_SIMULATE_PITOT         "simulate_pitot"
#define SETTINGS_SIMULATE_PITOT_FAILURE "simulate_pitot_failure"
#define SETTINGS_GRAPH_TYPE             "graph_type"
#define SETTINGS_MAP_MARKING_TYPE       "map_markings_type"
#define SETTINGS_VIDEOLINK_SIMULATION   "videolink_simulation"
#define SETTINGS_USE_HD_OSD             "HD-OSD"
#define SETTINGS_HD_FONT                "HD-Font"
#define SETTINGS_IS_HD                  "IS-HD"


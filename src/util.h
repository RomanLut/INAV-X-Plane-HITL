#pragma once

#include "config.h"

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

#ifdef LIN
extern bool IsDebuggerPresent();
extern uint32_t GetTickCount();
#endif
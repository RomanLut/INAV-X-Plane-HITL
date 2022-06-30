#pragma once

#include "config.h"

extern void setView();

extern void buildAssetFilename(char pName[MAX_PATH], const char* pFileName);
extern void playSound(const char* pFileName);

extern void LOG(const char* fmt, ...);

float latDistanceM(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2);

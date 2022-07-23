#pragma once

//#if IBM => WinAPI 
//#if LIN => Linux

#define ENABLE_LOG

#include <XPLMMenus.h>
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMMap.h>

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

#if IBM
#include <windows.h>
#define USE_OPENAL
#elif LIN
#include <unistd.h>
#define USE_OPENAL
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef LIN
#define MAX_PATH 1260
#include <time.h>
#include <cmath>
#endif



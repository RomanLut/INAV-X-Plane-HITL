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
#include <XPLMScenery.h>

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

#if IBM
#include <windows.h>
//do not use OpanAL on windows to avoid requirements of OpenAL installation
//#define USE_OPENAL
#elif LIN
#include <unistd.h>
#define USE_OPENAL
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>
//#include <iterator>
//#include <map>
#include <filesystem>
#include <fstream>
//#include <functional>


#include <GL/gl.h>
#include <GL/glu.h>

#include "mINI\ini.h"

#ifdef LIN
#define MAX_PATH 1260
#include <time.h>
#include <cmath>
#endif

#define HITL_VERSION_STRING "V1.3.1"

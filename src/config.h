#pragma once

//#if IBM => WinAPI 
//#if LIN => Linux

#define ENABLE_LOG

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <XPLMMenus.h>
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMMap.h>
#include <XPLMScenery.h>
#include <XPWidgetDefs.h>
#include <XPWidgets.h>
#include <XPStandardWidgets.h>

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

#if IBM
#include <windows.h>
#include <mmsystem.h> // Include mmsystem.h for PlaySound
//do not use OpanAL on windows to avoid requirements of OpenAL installation
//#define USE_OPENAL
#elif LIN
#include <unistd.h>
#define USE_OPENAL
#elif APL
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

#include <GL/glew.h>
#include "mINI/ini.h"

#if LIN || APL
#define MAX_PATH 1260
#include <time.h>
#include <cmath>
#endif

#if LIN || APL
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#if IBM
#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif



#define HITL_VERSION_STRING "V1.5.0"

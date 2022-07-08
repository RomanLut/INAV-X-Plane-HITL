#include "util.h"
#include "sound.h"

#include <math.h>
#include <stdarg.h>

#if LIN
#include <dlfcn.h>
#endif

//==============================================================
//==============================================================
void buildAssetFilename(char pName[MAX_PATH], const char* pFileName)
{
  char dirchar = *XPLMGetDirectorySeparator();
  XPLMGetPluginInfo(XPLMGetMyID(), NULL, pName, NULL, NULL);
  char* p = pName;
  char* slash = p;
  while (*p)
  {
    if (*p == dirchar) slash = p;
    ++p;
  }
  ++slash;
  *slash = 0;
  strcat(pName, pFileName);

  while (*slash)
  {
    if (*slash == '\\') *slash = dirchar;
    ++slash;
  }
}

//==============================================================
//==============================================================
void playSound(const char* pFileName)
{
  char assetName[MAX_PATH];

  buildAssetFilename(assetName, pFileName);

#ifdef USE_OPENAL
  g_sound.play(assetName);
#elif
  PlaySound(assetName, NULL, SND_ASYNC);
#endif
}

//==============================================================
//==============================================================
void LOG(const char* fmt, ...)
{
 #ifdef ENABLE_LOG

  va_list args;
  char msg[1024];

  snprintf(msg, 256, "INAVHITL[%ul]: ", GetTickCount());
  size_t hl = strlen(msg);

  va_start(args, fmt);
  vsnprintf(&msg[hl], 1024 - hl, fmt, args);
  va_end(args);
  msg[1023] = 0;

  strcat(msg, "\n");

  XPLMDebugString(msg);
#if IBM
  OutputDebugString(msg);
#endif

#endif
}

//==============================================================
//==============================================================
double toRad(double degree)
{
  return degree / 180 * 3.14159265358979323846;
}

//==============================================================
//==============================================================
float latDistanceM(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2)
{
  double dist;
  dist = sin(toRad(lat1)) * sin(toRad(lat2)) + cos(toRad(lat1)) * cos(toRad(lat2)) * cos(toRad(lon1 - lon2));
  dist = acos(dist);
  //        dist = (6371 * pi * dist) / 180;
  //got dist in radian, no need to change back to degree and convert to rad again.
  dist = 6371000 * dist;
  double dh = elev1 - elev2;
  dist = sqrt(dist*dist + dh * dh);
  return (float)dist;
}

//==============================================================
//==============================================================
void disableBrakes()
{
  //disable parking brakes
  XPLMDataRef df_parkBrake = XPLMFindDataRef("sim/flightmodel/controls/parkbrake");
  if (df_parkBrake != NULL)
  {
    XPLMSetDataf(df_parkBrake, 0);
  }
}


//==============================================================
//==============================================================
void setView()
{
  XPLMCommandRef command_ref = XPLMFindCommand("sim/view/forward_with_nothing");
  if (NULL != command_ref)
  {
    XPLMCommandOnce(command_ref);
  }

  //set FOV = 115
  XPLMDataRef df_fov = XPLMFindDataRef("sim/graphics/view/field_of_view_deg");
  if (df_fov != NULL)
  {
    XPLMSetDataf(df_fov, 110.0f);
  }

  //disable g load effects
  XPLMDataRef df_gload = XPLMFindDataRef("sim/graphics/settings/dim_gload");
  if (df_gload != NULL)
  {
    XPLMSetDatai(df_gload, 0);
  }
}

//==============================================================
//==============================================================
float clampf(float value, float minValue, float maxValue)
{
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  return value;
}

//==============================================================
//==============================================================
int16_t clampToInt16(float value)
{
  return (int16_t)round(clampf(value, INT16_MIN, INT16_MAX));
}

#ifdef LIN
//==============================================================
//==============================================================
bool IsDebuggerPresent()
{
	return false;
}

#endif

#ifdef LIN
//==============================================================
//==============================================================
uint32_t GetTickCount()
{
  enum
  {
#ifdef CLOCK_BOOTTIME
    boot_time_id = CLOCK_BOOTTIME
#else
    boot_time_id = 7
#endif
  };
  struct timespec spec;
  clock_gettime(boot_time_id, &spec);
  return (uint32_t)(((uint64_t)spec.tv_sec) * 1000 + ((uint64_t)spec.tv_nsec) / 1000000);
}
#endif

#if APL
//==============================================================
//==============================================================
// Mac specific: this converts file paths from HFS (which we get from the SDK) to Unix (which the OS wants).
// See this for more info:
// http://www.xsquawkbox.net/xpsdk/mediawiki/FilePathsAndMacho
static int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen) {

  CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath, kCFStringEncodingMacRoman);
  if (inStr == NULL)
    return -1;
  CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle, 0);
  CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
    return -1;
  CFRelease(outStr);
  CFRelease(url);
  CFRelease(inStr);
  return 0;
}
#endif


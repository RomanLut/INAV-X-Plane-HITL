#include "util.h"
#include <math.h>

//==============================================================
//==============================================================
void buildAssetFilename(char pName[MAX_PATH], const char* pFileName)
{
  HMODULE hm = NULL;

  if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    (LPCSTR)&buildAssetFilename, &hm) == 0)
  {
    return;
  }
  if (GetModuleFileName(hm, pName, MAX_PATH) == 0)
  {
    return;
  }

  char* dest = strstr(pName, "win.xpl");
  if (dest)
  {
    strcpy(dest, pFileName);
  }
}

//==============================================================
//==============================================================
void playSound(const char* pFileName)
{
  char assetName[MAX_PATH];

  buildAssetFilename(assetName, pFileName);
  PlaySound(assetName, NULL, SND_ASYNC);
}

//==============================================================
//==============================================================
void LOG(const char* fmt, ...)
{
 #ifdef ENABLE_LOG
  va_list args;
  char msg[1024];

  snprintf(msg, 256, "INAVHILT[%ul]: ", GetTickCount());
  size_t hl = strlen(msg);

  va_start(args, fmt);
  vsnprintf(&msg[hl], 1024 - hl, fmt, args);
  va_end(args);
  msg[1023] = 0;

  XPLMDebugString(msg);
  OutputDebugString(msg);
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
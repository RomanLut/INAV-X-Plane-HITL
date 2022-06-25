#include "util.h"

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
extern void LOG(const char* fmt, ...)
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
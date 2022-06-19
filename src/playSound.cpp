#include "windows.h"
#include <string.h>

//==============================================================
//==============================================================
void playSound(const char* pName)
{
  char path[MAX_PATH];
  HMODULE hm = NULL;

  if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    (LPCSTR)&playSound, &hm) == 0)
  {
    return;
  }
  if (GetModuleFileName(hm, path, sizeof(path)) == 0)
  {
    return;
  }

  char* dest = strstr(path, "win.xpl");
  if (dest)
  {
    strcpy(dest, pName);
    PlaySound(path, NULL, SND_ASYNC);
  }

}
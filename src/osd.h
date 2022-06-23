#pragma once

#include <windows.h>
#include <stdint.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <XPLMGraphics.h>
#include <XPLMDisplay.h>

#include <stdint.h>

#include "msp.h"

#define OSD_COLS      30
#define OSD_ROWS      16 // PAL
#define OSD_ROWS_NTSC 13 // NTSC

typedef enum
{
  OSD_NONE,
  OSD_AUTO,
  OSD_PAL,
  OSD_NTSC
} TOSDType;

//=======================================================
//=======================================================
class TOSD
{
public:

  TOSDType osd_type = OSD_AUTO;
  int auto_rows = 16;
  bool smoothed = true;

  void init();
  void destroy();

  void drawCallback();

  void updateFromINAV(const TMSPSimulatorFromINAV* message);

  void cbConnect(TCBConnectParm state);

private:
  int textureId = 0;

  uint16_t osdData[OSD_ROWS*OSD_COLS];

  void loadFont();

  void drawChar(uint16_t code, float x1, float y1, float width, float height);

  void clear();

  void drawString(int row, int col, const char* str);

  static int	drawCallbackStatic(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon);

};

extern TOSD g_osd;

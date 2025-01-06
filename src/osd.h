#pragma once


#include "msp.h"

#include "fontbase.h"
#include "osdRenderer.h"

#include <vector>


// PAL screen size
#define PAL_COLS 30
#define PAL_ROWS 16

// NTSC screen size
#define NTSC_COLS 30
#define NTSC_ROWS 13

// HDZERO screen size
#define HDZERO_COLS 50
#define HDZERO_ROWS 18

// Avatar screen size
#define AVATAR_COLS 53
#define AVATAR_ROWS 20

// DJIWTF screen size
#define DJI_COLS 60
#define DJI_ROWS 22

#define OSD_MAX_COLS      DJI_COLS
#define OSD_MAX_ROWS      DJI_ROWS

#define MAX7456_MODE_BLINK    (1 << 4)
#define MAKE_CHAR_MODE(c, m)    (MAKE_CHAR_MODE_U8(c, m) | (c > 255 ? CHAR_MODE_EXT : 0))
#define MAKE_CHAR_MODE_U8(c, m) ((((uint16_t)c) << 8) | m)
#define CHAR_MODE_EXT           (1 << 2)
#define CHAR_BYTE(x)            (x >> 8)
#define MODE_BYTE(x)            (x & 0xFF)
#define CHAR_IS_BLANK(x)        ((CHAR_BYTE(x) == 0x20 || CHAR_BYTE(x) == 0x00) && !CHAR_MODE_IS_EXT(MODE_BYTE(x)))
#define CHAR_MODE_IS_EXT(m)     ((m) & CHAR_MODE_EXT)

typedef enum
{
  OSD_AUTO = 1,
  OSD_PAL = 2,
  OSD_NTSC = 3
} TOSDType;

typedef enum
{
  VS_NONE,
  VS_2KM,
  VS_10KM,
  VS_50KM
} TVideoLinkSimulation;

//=======================================================
//=======================================================
class TOSD
{
public:

  bool visible = true;
  TOSDType osd_type = OSD_AUTO;
  bool smoothed = true;

  TVideoLinkSimulation videoLink = VS_50KM;

  void init();
  void destroy();

  void drawCallback();

  void updateFromINAV(const TMSPSimulatorFromINAV* message);
  void updateFromINAVOld(const TMSPSimulatorFromINAV* message);
  void updateFromINAVRowData(int osdRow, int osdCol, const uint8_t* data, int decodeRowsCount);

  void cbConnect(TCBConnectParm state);

  void setHome();

  void showMsg(const char* msg);

  void extractLatLon();

  void setHomeLocation(double home_lattitude, double home_longitude, double home_elevation);

  void disconnect();

  void loadConfig(mINI::INIStructure& ini);
  void saveConfig(mINI::INIStructure& ini);

  void addFontsToMenu();

  int getFontIndexByName(const char* name);

  void setActiveFontByIndex(int index);

private:
  int rows = 0;
  int cols = 0;

  int noiseTextureId = 0;
  int interferenceTextureId = 0;

  int textureWidth = 0;
  int textureHeight = 0;

  double home_lattitude = 0.0f;
  double home_longitude = 0.0f;
  double home_elevation = 0.0f;
  float roll = 0.0f;

  OsdRenderer *osdRenderer = nullptr;
  int noiseTexture = -1;
  int interferenceTexture = -1;

  uint16_t osdData[OSD_MAX_ROWS*OSD_MAX_COLS];

  int activeAnalogFontIndex = -1;
  int activeDigitalFontIndex = -1;
  unsigned int analogFontsCount = 0;
  std::vector<FontBase*> fonts;

  void loadFonts();
  void LoadFont(int index);

  void clear();

  void drawString(int row, int col, const char* str);

  void drawOSD();
  void drawNoise( float amount);
  void drawInterference( float amount );

  float getNoiseAmount();

  float extractFloat(int index);

  bool isAnalogOSD();
};

extern TOSD g_osd;

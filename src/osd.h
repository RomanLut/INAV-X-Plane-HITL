#pragma once

#include "config.h"

#include "msp.h"

#include "fontbase.h"

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
  int rows;
  int cols;

  int noiseTextureId = 0;
  int interferenceTextureId = 0;

  double home_lattitude;
  double home_longitude;
  double home_elevation;
  float roll;

  uint16_t osdData[OSD_MAX_ROWS*OSD_MAX_COLS];

  int activeAnalogFontIndex = 0;
  int activeDigitalFontIndex = 0;
  int analogFontsCount = 0;
  std::vector<FontBase*> fonts;

  void loadFonts();
  int loadTexture(const char* pFileName);

  FontBase* getActiveFont();

  void clear();

  void drawString(int row, int col, const char* str);

  void destroyTexture(int textureId);
  void drawOSD();
  void drawNoise( float amount);
  void drawInterference( float amount );

  float getNoiseAmount();

  float extractFloat(int index);

  bool isAnalogOSD();
};

extern TOSD g_osd;

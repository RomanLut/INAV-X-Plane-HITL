#pragma once

#include "config.h"

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

  TOSDType osd_type = OSD_AUTO;
  int auto_rows = 16;
  bool smoothed = true;

  TVideoLinkSimulation videoLink = VS_10KM;

  void init();
  void destroy();

  void drawCallback();

  void updateFromINAV(const TMSPSimulatorFromINAV* message);

  void cbConnect(TCBConnectParm state);

  void setHome();

  void showMsg(const char* msg);

  void extractLatLon();

  void setHomeLocation(double home_lattitude, double home_longitude, double home_elevation);

private:
  int fontTextureId = 0;
  int noiseTextureId = 0;
  int interferenceTextureId = 0;

  double home_lattitude;
  double home_longitude;
  double home_elevation;
  float roll;

  uint16_t osdData[OSD_ROWS*OSD_COLS];

  void loadFont();
  int loadTexture(const char* pFileName);

  void drawChar(uint16_t code, float x1, float y1, float width, float height);

  void clear();

  void drawString(int row, int col, const char* str);

  void destroyTexture(int textureId);
  void drawOSD();
  void drawNoise( float amount);
  void drawInterference( float amount );

  float getNoiseAmount();

  float extractFloat(int index);

};

extern TOSD g_osd;

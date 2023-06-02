#pragma once


#include "msp.h"
#include "osd_analog.h"
#include "osd_hd.h"

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
  bool smoothed = true;
  TVideoLinkSimulation videoLink = VS_50KM;
  std::string hdFont = std::string();

  void init();
  void destroy();
  void drawCallback();
  void updateFromINAV(const TMSPSimulatorFromINAV* message);
  void cbConnect(TCBConnectParm state);
  void setHDMode(bool enabeld);;
  void setFont(std::string font);
  std::string getFont();
  void setHome();
  void showMsg(char* msg);
  void setHomeLocation(double home_lattitude, double home_longitude, double home_elevation);
  void loadConfig(mINI::INIStructure& ini);
  void saveConfig(mINI::INIStructure& ini);

private:
  int noiseTextureId = 0;
  int interferenceTextureId = 0;
  bool isHD = false;

  double home_lattitude;
  double home_longitude;
  double home_elevation;
  float roll;

  std::unique_ptr<OSDBase> osdImpl = nullptr;

  int loadTexture(const char* pFileName);
  void clear();
  void drawString(int row, char* str);
  void drawNoise(float amount);
  void drawInterference(float amount);
  float getNoiseAmount();

};

extern TOSD g_osd;

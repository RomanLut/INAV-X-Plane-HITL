#pragma once

#include "osd_base.h"
#include "msp.h"

#define OSD_HD_COLS      60
#define OSD_HD_ROWS      22 // HD (WTFOS)

typedef enum
{
  OSD_STATE_Draw,
  OSD_STATE_SubCmd,
  OSD_STATE_PositionX,
  OSD_STATE_PositionY,
  OSD_STATE_StringInfo,
  OSD_STATE_String
}OsdPayloadState;

class OSD_HD : public OSDBase
{
private:
  uint16_t screen[OSD_HD_ROWS][OSD_HD_COLS] = { 0 };
  std::vector<int> textureIDs;
  std::string font;
  OsdPayloadState state = OSD_STATE_SubCmd;

  void destroyFontTextures(void);
  void drawChar(int c, double left, double top, double right, double bottom, bool smoothed);
  bool loadFontFile(std::string path);
public:
  OSD_HD();
  ~OSD_HD();
  void decode(const TMSPSimulatorFromINAV* message);
  void disconnect(void);
  void draw(TOSDType osd_type, bool smoothed);
  bool loadFont(void);
  void setFont(std::string fontName);
  std::string getFont();
  void destroy(void);
  void clear(void);
  uint16_t getChar(int row, int col);
  void setChar(int row, int col, char c);
  int getColCount();
  int getRowCount();
};


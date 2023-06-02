#pragma once

#include "osd_base.h"

#define OSD_COLS      30
#define OSD_ROWS      16 // PAL
#define OSD_ROWS_NTSC 13 // NTSC


class OSDAnalog : public OSDBase
{
private:
  int fontTextureId = 0;
  uint16_t osdData[OSD_ROWS * OSD_COLS];
  int auto_rows = 16;

  void drawChar(uint16_t code, float x1, float y1, float width, float height);
public:
  ~OSDAnalog();
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


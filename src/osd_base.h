#pragma once

#include "config.h"
#include "msp.h"

#define CHAR_BYTE(x) (x >> 8)

typedef enum 
{
  OSD_NONE,
  OSD_AUTO,
  OSD_PAL,
  OSD_NTSC,
}TOSDType;

class OSDBase
{
public:
  virtual ~OSDBase() {};
  virtual void disconnect(void) = 0;
  virtual void decode(const TMSPSimulatorFromINAV* message) = 0;
  virtual void draw(TOSDType osd_type, bool smoothed) = 0;
  virtual bool loadFont(void) = 0;
  virtual void setFont(std::string fontName) = 0;
  virtual std::string getFont() = 0;
  virtual void destroy(void) = 0;
  virtual void clear(void) = 0;
  virtual uint16_t getChar(int row, int col) = 0;
  virtual void setChar(int row, int col, char c) = 0;
  virtual int getColCount() = 0;
  virtual int getRowCount() = 0;
protected:
  float extractFloat(int x, int y);
  void extractLatLon(void);
};


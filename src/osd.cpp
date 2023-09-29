
#include "lodepng.h"

#include "osd.h"
#include "util.h"
#include "stats.h"
#include "msp.h"
#include "map.h"

#include "menu.h"

#include "fontanalog.h"

TOSD g_osd;


#define MAX7456_MODE_INVERT   (1 << 3)
#define MAX7456_MODE_BLINK    (1 << 4)
#define MAX7456_MODE_SOLID_BG (1 << 5)

#define CHAR_MODE_EXT           (1 << 2)
#define MAKE_CHAR_MODE_U8(c, m) ((((uint16_t)c) << 8) | m)
#define MAKE_CHAR_MODE(c, m)    (MAKE_CHAR_MODE_U8(c, m) | (c > 255 ? CHAR_MODE_EXT : 0))
#define CHAR_BYTE(x)            (x >> 8)
#define MODE_BYTE(x)            (x & 0xFF)
#define CHAR_IS_BLANK(x)        ((CHAR_BYTE(x) == 0x20 || CHAR_BYTE(x) == 0x00) && !CHAR_MODE_IS_EXT(MODE_BYTE(x)))
#define CHAR_MODE_IS_EXT(m)     ((m) & CHAR_MODE_EXT)

#define OSD_MARGIN_HOR_PERCENT      10
#define OSD_MARGIN_VERT_PERCENT     3

#define NOISE_TEXTURE_WIDTH   1024
#define NOISE_TEXTURE_HEIGHT  1024

#define INERFERENCE_TEXTURE_WIDTH   1024
#define INERFERENCE_TEXTURE_HEIGHT  128

#define SYM_LAT                     0x03  // 003 GPS LAT
#define SYM_LON                     0x04  // 004 GPS LON
#define SYM_ZERO_HALF_TRAILING_DOT  0xA1  // 161 to 170 Numbers with trailing dot
#define SYM_ZERO_HALF_LEADING_DOT   0xB1  // 177 to 186 Numbers with leading dot

//==============================================================
//==============================================================
void TOSD::drawOSD()
{
  if (!this->visible) return;
  if (this->rows == 0) return;

  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  this->getActiveFont()->bindTexture();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->smoothed ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->smoothed ? GL_LINEAR : GL_NEAREST);

  // The drawing part.
  XPLMSetGraphicsState(
    0,        // No fog, equivalent to glDisable(GL_FOG);
    1,        // One texture, equivalent to glEnable(GL_TEXTURE_2D);
    0,        // No lighting, equivalent to glDisable(GL_LIGHT0);
    0,        // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
    1,        // Use alpha blending, e.g. glEnable(GL_BLEND);
    0,        // No depth read, e.g. glDisable(GL_DEPTH_TEST);
    0);        // No depth write, e.g. glDepthMask(GL_FALSE);

  glColor3f(1, 1, 1);        // Set color to white.

  uint32_t t = GetTickCount();
  bool blink = (GetTickCount() % 266) < 133;

  /*
  int x1 = 0;
  int y1 = sy;
  int x2 = TEXTURE_WIDTH;
  int y2 = sy - TEXTURE_HEIGHT ;

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);        glVertex2f(x1, y1);        // We draw one textured quad.  Note: the first numbers 0,1 are texture coordinates, which are ratios.
  glTexCoord2f(1, 0);        glVertex2f(x2, y1);        // unless you change it; if you change it, change it back!
  glTexCoord2f(1, 1);        glVertex2f(x2, y2);        // would use 0,0 to 0,0.5 to 1,0.5, to 1,0.  Note that for X-Plane front facing polygons are clockwise
  glTexCoord2f(0, 1);        glVertex2f(x1, y2);        // lower left is 0,0, upper right is 1,1.  So if we wanted to use the lower half of the texture, we
  glEnd();
  */

  int rowsCount = this->rows;

  if (isAnalogOSD())
  {
    if (this->osd_type == OSD_NTSC)
    {
      rowsCount = NTSC_ROWS;
    }
    else if (this->osd_type == OSD_PAL)
    {
      rowsCount = PAL_ROWS;
    }
  }

  float marginX = sx * OSD_MARGIN_HOR_PERCENT / 100.0f;
  float marginY = sy * OSD_MARGIN_VERT_PERCENT / 100.0f;

  /*
  if (rowsCount == NTSC_ROWS)
  {
    marginY += (sy - marginY * 2) / rowsCount;
  }
  */

  float x0 = marginX;
  float y0 = marginY;

  float cw = ( sx - marginX * 2 ) / this->cols;
  float ch = ( sy - marginY * 2 ) / rowsCount;

  //a cross on 30 cols osd does not aling ok to the center of the screen
  //shift half charwidth to the left to aling crosss to screen center
  x0 -= cw / 2; 

  glBegin(GL_QUADS);

  for (int y = 0; y < rowsCount; y++)
  {
    for (int x = 0; x < this->cols; x++)
    {
      //int icode = (x*y) & 511;
      //int code = MAKE_CHAR_MODE(icode, 0);

      int code = this->osdData[y * OSD_MAX_COLS + x];

      if ( CHAR_IS_BLANK( code )) continue;

      if (blink && ((MAX7456_MODE_BLINK & code) != 0) ) continue;

      int code9 = CHAR_BYTE(code) | (code & CHAR_MODE_EXT ? 0x100 : 0);
      this->getActiveFont()->drawChar(code9, x0 + x * cw, sy - (y0 + y * ch), cw, ch);
    }
  }

  glEnd();
}

//==============================================================
//==============================================================
void TOSD::drawNoise( float amount)
{
  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);                          

  XPLMBindTexture2d(this->noiseTextureId, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  XPLMSetGraphicsState(
    0,        // No fog, equivalent to glDisable(GL_FOG);
    1,        // One texture, equivalent to glEnable(GL_TEXTURE_2D);
    0,        // No lighting, equivalent to glDisable(GL_LIGHT0);
    0,        // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
    1,        // Use alpha blending, e.g. glEnable(GL_BLEND);
    0,        // No depth read, e.g. glDisable(GL_DEPTH_TEST);
    0);        // No depth write, e.g. glDepthMask(GL_FALSE);

  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);

  float f = 1.0f;
  glColor4f(f, f, f, amount * amount * amount * amount);       

  float size = sx * 1.2f;

  static float dx = 0;
  static float dy = 0;        
  static uint32_t t = GetTickCount();

  uint32_t t1 = GetTickCount();
  if ((t1 - t) > 40)
  {
    t = t1;
    dx = -(float)(size - sx) * rand() / RAND_MAX;
    dy = -(float)(size - sy) * rand() / RAND_MAX;
    size = size + size * rand() / RAND_MAX;
  }

  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);        glVertex2f(dx, (float)(dy + size));
  glTexCoord2f(1, 1);        glVertex2f((float)(dx + size), (float)(dy + size));
  glTexCoord2f(1, 0);        glVertex2f((float)(dx + size), dy);
  glTexCoord2f(0, 0);        glVertex2f(dx, dy);
  glEnd();
}

//==============================================================
//==============================================================
void TOSD::drawInterference(float amount)
{
  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  XPLMBindTexture2d(this->interferenceTextureId, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  XPLMSetGraphicsState(
    0,        // No fog, equivalent to glDisable(GL_FOG);
    1,        // One texture, equivalent to glEnable(GL_TEXTURE_2D);
    0,        // No lighting, equivalent to glDisable(GL_LIGHT0);
    0,        // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
    1,        // Use alpha blending, e.g. glEnable(GL_BLEND);
    0,        // No depth read, e.g. glDisable(GL_DEPTH_TEST);
    0);        // No depth write, e.g. glDepthMask(GL_FALSE);

  glColor4f(1.0f, 1.0f, 1.0f, ( amount ));

  float sizeX = sx * 1.2f;
  float sizeY = sx * 1.2f / INERFERENCE_TEXTURE_WIDTH * INERFERENCE_TEXTURE_HEIGHT;

  static float dx = 0;
  static float dy = 0;
  static uint32_t t = GetTickCount();
  static float delay = 1;

  uint32_t t1 = GetTickCount();
  if ((t1 - t) > 40)
  {
    if ((t1 - t) < (((1.0f - amount) * delay) * 3000.0f))
    {
      dy = 10000;
      return;
    }

    t = GetTickCount();
    dx = -(float)(sizeX - sx) * rand() / RAND_MAX;
    dy = (float)sy * rand() / RAND_MAX;

    sizeX = sizeX + sizeX * rand() / RAND_MAX;
    sizeY = sizeY * ( rand() / RAND_MAX + 0.3f );

    if (1.0f * rand() / RAND_MAX > ( pow( amount, 0.25 ) )) dy = 10000;

    delay = 2.0f * rand() / RAND_MAX;
  }

  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);        glVertex2f(dx, (float)(dy + sizeY));
  glTexCoord2f(1, 1);        glVertex2f((float)(dx + sizeX), (float)(dy + sizeY));
  glTexCoord2f(1, 0);        glVertex2f((float)(dx + sizeX), dy);
  glTexCoord2f(0, 0);        glVertex2f(dx, dy);
  glEnd();
}

//==============================================================
//==============================================================
void TOSD::drawCallback()
{
  this->drawOSD();

  if ((this->videoLink != VS_NONE) && g_msp.isConnected() && this->isAnalogOSD())
  {
    float amount = this->getNoiseAmount();
    this->drawNoise(amount);
    this->drawInterference(amount);
  }
}

//==============================================================
//==============================================================
FontBase* TOSD::getActiveFont()
{
  if (this->isAnalogOSD())
  {
    return this->fonts[this->activeAnalogFontIndex];
  }
  else
  {
    return this->fonts[this->activeDigitalFontIndex];
  }
}

//==============================================================
//==============================================================
void TOSD::loadFonts()
{
  char assetFileName[MAX_PATH];

  //load analog fonts
  std::vector<std::filesystem::path> fontPaths = getFontPaths("assets\\fonts\\analog\\", false);
  for (auto fontEntry : fontPaths)
  {
    if (toLower(fontEntry.extension().string()) == ".png")
    {
      buildAssetFilename(assetFileName, ("assets\\fonts\\analog\\" + fontEntry.filename().string()).c_str());
      char fontName[64];
      strcpy(fontName, fontEntry.filename().string().c_str());
      *strstr(fontName, ".") = 0;
      this->fonts.push_back(new FontAnalog(assetFileName, fontName));
    }
  }

  this->analogFontsCount = (int)this->fonts.size();
}

//==============================================================
//==============================================================
 int TOSD::loadTexture( const char* pFileName )
{
  char assetFileName[MAX_PATH];

  buildAssetFilename(assetFileName, pFileName);

  unsigned char* image = 0;
  unsigned width, height;

  unsigned int error = lodepng_decode32_file(&image, &width, &height, assetFileName);
  if (error)
  {
    LOG("error %u: %s\n", error, lodepng_error_text(error));
    return 0;
  }

  int res;
  XPLMGenerateTextureNumbers(&res, 1);
  XPLMBindTexture2d(res, 0);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,                   // mipmap level
    GL_RGBA,             // internal format for the GL to use.  (We could ask for a floating point tex or 16-bit tex if we were crazy!)
    width,
    height,
    0,                   // border size
    GL_RGBA,             // format of color we are giving to GL
    GL_UNSIGNED_BYTE,    // encoding of our data
    image);

  free(image);

  return res;
}

//==============================================================
//==============================================================
void TOSD::init()
{
  this->loadFonts();

  this->noiseTextureId = this->loadTexture("assets\\noise.png");
  this->interferenceTextureId = this->loadTexture("assets\\interference.png");

  this->rows = PAL_ROWS;
  this->cols = PAL_COLS;

  this->cbConnect(CBC_DISCONNECTED);
}

//==============================================================
//==============================================================
void TOSD::destroyTexture(int textureId)
{
  XPLMBindTexture2d(textureId, 0);
  GLuint t = textureId;
  glDeleteTextures(1, &t);
}

//==============================================================
//==============================================================
void TOSD::destroy()
{
  for (FontBase* pFont : this->fonts)
  {
    pFont->destroy();
  }

  this->destroyTexture(this->noiseTextureId);
}

//==============================================================
//==============================================================
void TOSD::updateFromINAV(const TMSPSimulatorFromINAV* message)
{
  if (message->newFormat.newFormatSignature != 255)
  {
    this->updateFromINAVOld(message);
    return;
  }

  if (message->newFormat.osdRows == 0 )
  {
    return; //old format nodata [255, unfilled] or new format no data [255, 0 ]
  }

  int formatVersion = (message->newFormat.osdRows >> 5) & 7;

  if (formatVersion != 0)
  {
    return;
  }

  int message_osdRows = message->newFormat.osdRows & 0x1f;
  int message_osdCols = message->newFormat.osdCols & 0x3f;

  if (message_osdRows > OSD_MAX_ROWS)
  {
    return; //invalid data
  }

  if (message_osdCols > OSD_MAX_COLS)
  {
    return; //invalid data
  }

  this->rows = message_osdRows;
  this->cols = message_osdCols;

  int osdRow = message->newFormat.osdRow & 0x1f;
  int osdCol = message->newFormat.osdCol & 0x3f;

  this->updateFromINAVRowData(osdRow, osdCol, message->newFormat.osdRowData, this->rows);
}

//==============================================================
//==============================================================
void TOSD::updateFromINAVOld(const TMSPSimulatorFromINAV* message)
{
  this->rows = (message->oldFormat.osdRow & 0x80) ? PAL_ROWS : NTSC_ROWS;
  this->cols = PAL_COLS;

  int osdRow = message->oldFormat.osdRow & 0x7f;
  int osdCol = message->oldFormat.osdCol;

  this->updateFromINAVRowData(osdRow, osdCol, message->oldFormat.osdRowData, 16);
}

//==============================================================
//==============================================================
void TOSD::updateFromINAVRowData(int osdRow, int osdCol, const uint8_t* data, int decodeRowsCount)
{
  if (osdRow >= this->rows)
  {
    return; //invalid data
  }

  if (osdCol >= this->cols)
  {
    return; //invalid data
  }

  bool highBank = false;
  bool blink = false;
  int count;

  int byteCount = 0;
  while (byteCount < (400 - 3 - 2))
  {
    uint8_t c = data[byteCount++];
    if (c == 0)
    {
      c = data[byteCount++];
      count = (c & 0x3f);
      if (count == 0)
      {
        break; //stop
      }
      highBank ^= (c & 64) != 0;
      blink ^= (c & 128) != 0;
      c = data[byteCount++];
    }
    else if (c == 255)
    {
      highBank = !highBank;
      count = 0;
    }
    else
    {
      count = 1;
    }

    while (count > 0)
    {
      this->osdData[osdRow * OSD_MAX_COLS + osdCol] = MAKE_CHAR_MODE((c | (highBank ? 0x100 : 0)), (blink ? MAX7456_MODE_BLINK : 0));
      osdCol++;
      if (osdCol == this->cols)
      {
        osdCol = 0;
        osdRow++;
        if (osdRow == decodeRowsCount)
        {
          this->extractLatLon();
          osdRow = 0;
          g_stats.OSDUpdates++;
        }
      }
      count--;
    }
  }
}

//==============================================================
//==============================================================
void TOSD::clear()
{
  memset(this->osdData, 0, OSD_MAX_ROWS*OSD_MAX_COLS * 2);
}

//==============================================================
//==============================================================
void TOSD::drawString( int row, int col, const char* str)
{
  uint16_t* p = &this->osdData[ row * OSD_MAX_COLS + col];

  while (*str && p < &this->osdData[OSD_MAX_ROWS * OSD_MAX_COLS ])
  {
    char c = *str++;  //avoid macro side effect
    if (c == '\n')
    {
      row++;
      p = &this->osdData[row * OSD_MAX_COLS + col];
    }
    else
    {
      *p++ = MAKE_CHAR_MODE(c, 0);
    }
  }

}

//==============================================================
//==============================================================
void TOSD::setHome()
{
  g_simData.updateFromXPlane();
  this->home_lattitude = g_simData.lattitude;;
  this->home_longitude = g_simData.longitude;
  this->home_elevation = g_simData.elevation;
  this->roll = g_simData.roll;
}

//==============================================================
//==============================================================
void TOSD::cbConnect(TCBConnectParm state)
{
  this->clear();

  if (state == CBC_CONNECTED)
  {
    this->setHome();
  }

  if (state == CBC_CONNECTION_FAILED)
  {
    g_osd.disconnect();
    g_osd.showMsg("CONNECTION FAILED");
  }
  else if (state == CBC_TIMEOUT_DISCONNECTED)
  {
    g_osd.disconnect();
    g_osd.showMsg("CONNECTION LOST");
  }
  else  if (state != CBC_CONNECTED)
  {
    g_osd.disconnect();
    this->drawString(0, 4, "INAV HITL DISCONNECTED");
    this->drawString(1, 15 - (int)(strlen(HITL_VERSION_STRING)) / 2 , HITL_VERSION_STRING);
  }
}

//==============================================================
//==============================================================
float TOSD::getNoiseAmount()
{
  float d = latDistanceM(this->home_lattitude, this->home_longitude, this->home_elevation,
    g_simData.lattitude, g_simData.longitude, g_simData.elevation);

  float maxD = 50000;
  switch (this->videoLink)
  {
  case VS_2KM:
    maxD = 2000;
    break;
  case VS_10KM:
    maxD = 10000;
    break;
  case VS_50KM:
    maxD = 50000;
    break;
  }

  float res = d /maxD;
  float s = sin(g_simData.roll / 180.0f*3.14f);
  res += s*s * 0.2f;
  if (res > 0.99f) res = 0.99f;

  if (res < 0.475f) res = 0.475f;    
  return res;
}

//==============================================================
//==============================================================
void TOSD::showMsg(const char* msg)
{
  this->clear();
  this->drawString(0, 4, msg);
}

//==============================================================
//==============================================================
float TOSD::extractFloat(int index)
{
  int rowEndIndex = (index / OSD_MAX_COLS) * OSD_MAX_COLS + this->cols;

  int res = 0;
  bool neg = false;
  int div = 1;
  bool p = false;

  index++;

  while (index < rowEndIndex)
  {
    int ch = CHAR_BYTE(this->osdData[index]);

    if ( ch == '-')
    {
      neg = true;
    }
    else if ((ch >= '0') && (ch <= '9'))
    {
      res = res * 10 + (ch - '0');
    }
    else if ((ch >= SYM_ZERO_HALF_TRAILING_DOT) && (ch <= (SYM_ZERO_HALF_TRAILING_DOT+10)))
    {
      res = res * 10 + (ch - SYM_ZERO_HALF_TRAILING_DOT);
    }
    else if ((ch >= SYM_ZERO_HALF_LEADING_DOT) && (ch <= (SYM_ZERO_HALF_LEADING_DOT + 10)))
    {
      res = res * 10 + (ch - SYM_ZERO_HALF_LEADING_DOT);
      p = true;
    }
    else
    {
      break;
    }
      
    index++;
    if (p) div *= 10;
  }

  float resf = res * 1.0f / div;

  return neg ? -resf : resf;
}

//==============================================================
//==============================================================
void TOSD::extractLatLon()
{
  float lat = 0;
  float lon = 0;

  int flags = 0;

  for (int i = 0; i < OSD_MAX_ROWS*OSD_MAX_COLS; i++)
  {
    if (CHAR_BYTE(osdData[i]) == SYM_LAT)
    {
      lat = this->extractFloat(i);
      flags |= 1;
    }
    if (CHAR_BYTE(osdData[i]) == SYM_LON)
    {
      lon = this->extractFloat(i);
      flags |= 2;
    }
  }

  if (flags == 3)
  {
    g_map.addLatLonOSD(lat, lon);
  }
}

//==============================================================
//==============================================================
void TOSD::setHomeLocation(double home_lattitude, double home_longitude, double home_elevation)
{
  this->home_lattitude = home_lattitude;
  this->home_longitude = home_longitude;
  this->home_elevation = home_elevation;
}

//==============================================================
//==============================================================
void TOSD::loadConfig(mINI::INIStructure& ini)
{
  this->visible = !ini[SETTINGS_SECTION].has(SETTINGS_OSD_SMOOTHED) || (ini[SETTINGS_OSD_SMOOTHED][SETTINGS_OSD_SMOOTHED] != "0");


  if (ini[SETTINGS_SECTION].has(SETTINGS_OSD_TYPE))
  {
    this->osd_type = (TOSDType)atoi(ini[SETTINGS_SECTION][SETTINGS_OSD_TYPE].c_str());
    if (this->osd_type < OSD_AUTO || this->osd_type > OSD_NTSC)
    {
      this->osd_type = OSD_AUTO;
    }
  }

  this->smoothed = !ini[SETTINGS_SECTION].has(SETTINGS_OSD_SMOOTHED) || (ini[SETTINGS_OSD_SMOOTHED][SETTINGS_OSD_SMOOTHED] != "0");

  if (ini[SETTINGS_SECTION].has(SETTINGS_VIDEOLINK_SIMULATION))
  {
    this->videoLink = (TVideoLinkSimulation)atoi(ini[SETTINGS_SECTION][SETTINGS_VIDEOLINK_SIMULATION].c_str());
    if (this->videoLink < VS_NONE || this->videoLink > VS_50KM)
    {
      this->videoLink = VS_50KM;
    }
  }

  this->activeAnalogFontIndex = 0;
  const char* c = ini[SETTINGS_SECTION][SETTINGS_ANALOG_OSD_FONT].c_str();
  for (int i = 0; i < this->analogFontsCount; i++)
  {
    if (strcmp( this->fonts[i]->name, c) == 0)
    {
      this->activeAnalogFontIndex = i;
      break;
    }
  }

  this->activeDigitalFontIndex = this->analogFontsCount;
  c = ini[SETTINGS_SECTION][SETTINGS_DIGITAL_OSD_FONT].c_str();
  for (int i = analogFontsCount; i < this->fonts.size(); i++)
  {
    if (strcmp(this->fonts[i]->name, c) == 0)
    {
      this->activeDigitalFontIndex = i;
      break;
    }
  }

  g_menu.updateFontsMenu(this->activeAnalogFontIndex, this->activeDigitalFontIndex);
}

//==============================================================
//==============================================================
void TOSD::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_OSD_TYPE] = std::to_string(this->osd_type);
  ini[SETTINGS_SECTION][SETTINGS_OSD_SMOOTHED] = std::to_string(this->smoothed ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_VIDEOLINK_SIMULATION] = std::to_string(this->videoLink);

  ini[SETTINGS_SECTION][SETTINGS_ANALOG_OSD_FONT] = std::string(this->fonts[this->activeAnalogFontIndex]->name);
  ini[SETTINGS_SECTION][SETTINGS_DIGITAL_OSD_FONT] = std::string(this->fonts[this->activeAnalogFontIndex]->name);
}

//==============================================================
//==============================================================
bool TOSD::isAnalogOSD()
{
  return (this->cols == 30) && ((this->rows == 13) || (this->rows == 16));
}

//==============================================================
//==============================================================
void TOSD::disconnect()
{
  this->cols = PAL_COLS;
  this->rows = PAL_ROWS;
}

//==============================================================
//==============================================================
void TOSD::addFontsToMenu()
{
  for (auto font : this->fonts)
  {
    g_menu.addFontEntry(font->isAnalog(), font->name);
  }
}

//==============================================================
//==============================================================
int TOSD::getFontIndexByName(const char* name)
{
  for (int i = 0; i < this->fonts.size(); i++)
  {
    if (strcmp(name, this->fonts[i]->name) == 0) return i;
  }
  return -1;
}

//==============================================================
//==============================================================
void TOSD::setActiveFontByIndex(int index)
{
  if (this->fonts[index]->isAnalog())
  {
    this->activeAnalogFontIndex = index;
  }
  else
  {
    this->activeDigitalFontIndex = index;
  }

  g_menu.updateFontsMenu(this->activeAnalogFontIndex, this->activeDigitalFontIndex);
}

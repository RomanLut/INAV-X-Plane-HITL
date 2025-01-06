

#include "osd.h"
#include "util.h"
#include "stats.h"
#include "msp.h"
#include "map.h"

#include "menu.h"

#include "fontanalog.h"
#include "fontwalksnail.h"
#include "fonthdzero.h"
#include "fontwtfos.h"

TOSD g_osd;

#define OSD_MARGIN_PERCENT      1

#define NOISE_TEXTURE_WIDTH   1024
#define NOISE_TEXTURE_HEIGHT  1024

#define INERFERENCE_TEXTURE_WIDTH   1024
#define INERFERENCE_TEXTURE_HEIGHT  128

#define SYM_LAT                     0x03  // 003 GPS LAT
#define SYM_LON                     0x04  // 004 GPS LON
#define SYM_ZERO_HALF_TRAILING_DOT  0xA1  // 161 to 170 Numbers with trailing dot
#define SYM_ZERO_HALF_LEADING_DOT   0xB1  // 177 to 186 Numbers with leading dot

#define OSD_MARGIN 30

//==============================================================
//==============================================================
void TOSD::drawOSD()
{
  if (!this->visible) return;
  if (this->rows == 0) return;

  int windowWidth, windowHeight;
  XPLMGetScreenSize(&windowWidth, &windowHeight);

  uint32_t t = GetTicks();
  bool blink = (GetTicks() % 266) < 133;

  const float textureAspectRatio = static_cast<float>(this->textureWidth) / static_cast<float>(this->textureHeight);

  int cellWidth = windowWidth / this->textureWidth;
  int cellHeight = static_cast<int>(windowHeight / textureAspectRatio);

  const int avaiableWidth = windowWidth - 2 * OSD_MARGIN;
  const int avaiableHeight = windowHeight - 2 * OSD_MARGIN;

  if (cellWidth * this->cols > avaiableWidth) {
    cellWidth = avaiableWidth / this->cols;
    cellHeight = static_cast<int>(cellWidth / textureAspectRatio);
  }

  if (cellHeight * this->rows > avaiableHeight) {
    cellHeight = avaiableHeight / this->rows;
    cellWidth = static_cast<int>(cellHeight * textureAspectRatio);
  }

  int xOffset = (windowWidth - cellWidth * cols) / 2;
  int yOffset = (windowHeight - cellHeight * rows) / 2;

  this->osdRenderer->drawOSD(this->osdData, this->rows, this->cols, cellWidth, cellHeight, xOffset, yOffset, blink);
}

//==============================================================
//==============================================================
void TOSD::drawNoise( float amount)
{
  if (this->noiseTexture < 0) {
    return;
  }

  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  float size = sx * 1.2f;

  static float dx = 0;
  static float dy = 0;        
  static uint32_t t = GetTicks();

  uint32_t t1 = GetTicks();
  if ((t1 - t) > 40)
  {
    t = t1;
    dx = -(float)(size - sx) * rand() / RAND_MAX;
    dy = -(float)(size - sy) * rand() / RAND_MAX;
    size = size + size * rand() / RAND_MAX;
  }

  this->osdRenderer->drawInterferenceTexture(this->noiseTexture, static_cast<int>(dx), static_cast<int>(dy), static_cast<int>(size), static_cast<int>(size), amount * amount * amount * amount);
}

//==============================================================
//==============================================================
void TOSD::drawInterference(float amount)
{
  if (this->interferenceTexture < 0) {
    return;
  }

  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  float sizeX = sx * 1.2f;
  float sizeY = sx * 1.2f / INERFERENCE_TEXTURE_WIDTH * INERFERENCE_TEXTURE_HEIGHT;

  static float dx = 0;
  static float dy = 0;
  static uint32_t t = GetTicks();
  static float delay = 1;

  uint32_t t1 = GetTicks();
  if ((t1 - t) > 40)
  {
    if ((t1 - t) < (((1.0f - amount) * delay) * 3000.0f))
    {
      dy = 10000;
      return;
    }

    t = GetTicks();
    dx = -(float)(sizeX - sx) * rand() / RAND_MAX;
    dy = (float)sy * rand() / RAND_MAX;

    sizeX = sizeX + sizeX * rand() / RAND_MAX;
    sizeY = sizeY * ( rand() / RAND_MAX + 0.3f );

    if (1.0f * rand() / RAND_MAX > ( pow( amount, 0.25 ) )) dy = 10000;

    delay = 2.0f * rand() / RAND_MAX;
  }

  this->osdRenderer->drawInterferenceTexture(this->interferenceTexture, static_cast<int>(dx), static_cast<int>(dy), static_cast<int>(sizeX), static_cast<int>(sizeY), amount);
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
      this->fonts.push_back(new FontAnalog(std::string(assetFileName)));
    }
  }

  this->analogFontsCount = (unsigned int)this->fonts.size();

  //load hdzero fonts
  fontPaths = getFontPaths("assets\\fonts\\digital\\hdzero\\", false);
  for (auto& fontEntry : fontPaths)
  {
    if (toLower(fontEntry.extension().string()) == ".bmp")
    {
      buildAssetFilename(assetFileName, ("assets\\fonts\\digital\\hdzero\\" + fontEntry.filename().string()).c_str());
      this->fonts.push_back(new FontHDZero(std::string(assetFileName)));
    }
  }

  //load walksnail fonts
  fontPaths = getFontPaths("assets\\fonts\\digital\\walksnail\\", false);
  for (auto& fontEntry : fontPaths)
  {
    if (toLower(fontEntry.extension().string()) == ".png")
    {
      buildAssetFilename(assetFileName, ("assets\\fonts\\digital\\walksnail\\" + fontEntry.filename().string()).c_str());
      this->fonts.push_back(new FontWalksnail(std::string(assetFileName)));
    }
  }

  //load wtfos fonts
  fontPaths = getFontPaths("assets\\fonts\\digital\\wtfos\\", true);
  for (auto& fontEntry : fontPaths)
  {
    buildAssetFilename(assetFileName, ("assets\\fonts\\digital\\wtfos\\" + fontEntry.filename().string()).c_str());
    this->fonts.push_back(new FontWtfOS(std::string(assetFileName)));
  }

}

void TOSD::LoadFont(int index)
{
  FontBase* font = this->fonts[index];
  osdRenderer->loadOSDTextures(font->getTextures(), font->getCharWidth(), font->getCharHeight(), this->smoothed);
  this->textureWidth = font->getCharWidth();
  this->textureHeight = font->getCharHeight();
}


//==============================================================
//==============================================================
void TOSD::init()
{
  this->loadFonts();

  this->rows = PAL_ROWS;
  this->cols = PAL_COLS;

  this->cbConnect(CBC_DISCONNECTED);

  if (glewInit() != GLEW_OK) {
    LOG("Unable to init GLEW");
  }

  this->osdRenderer = new OsdRenderer();

  char assetFileName[MAX_PATH];
  buildAssetFilename(assetFileName, "assets\\noise.png");
  int id = this->osdRenderer->loadInterferenceTexture(assetFileName, true);
  if (id >= 0) {
    this->noiseTexture = id;
  }

  buildAssetFilename(assetFileName, "assets\\interference.png");
  id = this->osdRenderer->loadInterferenceTexture(assetFileName, true);
  if (id >= 0) {
    this->interferenceTexture = id;
  }
}

//==============================================================
//==============================================================
void TOSD::destroy()
{
  delete this->osdRenderer;
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
      c = data[byteCount++];
      count = 1;
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
  for (unsigned int i = 0; i < this->analogFontsCount; i++)
  {
    if (strcmp( this->fonts[i]->name, c) == 0)
    {
      this->activeAnalogFontIndex = i;
      break;
    }
  }

  this->activeDigitalFontIndex = this->analogFontsCount;

  for (int i = this->analogFontsCount; i < this->fonts.size(); i++)
  {
    if (strcmp(this->fonts[i]->name, "Walksnail: INAV_default_36") == 0)
    {
      this->activeDigitalFontIndex = i;
      break;
    }
  }

  c = ini[SETTINGS_SECTION][SETTINGS_DIGITAL_OSD_FONT].c_str();
  for (int i = this->analogFontsCount; i < this->fonts.size(); i++)
  {
    if (strcmp(this->fonts[i]->name, c) == 0)
    {
      this->activeDigitalFontIndex = i;
      break;
    }
  }

  this->LoadFont(this->activeDigitalFontIndex);
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
  ini[SETTINGS_SECTION][SETTINGS_DIGITAL_OSD_FONT] = std::string(this->fonts[this->activeDigitalFontIndex]->name);
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

  this->LoadFont(index);
  g_menu.updateFontsMenu(this->activeAnalogFontIndex, this->activeDigitalFontIndex);
}

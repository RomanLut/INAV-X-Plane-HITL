
#include "osd.h"
#include "util.h"
#include "msp.h"
#include "map.h"
#include "lodepng.h"
#include "simData.h"

TOSD g_osd;

#define NOISE_TEXTURE_WIDTH   1024
#define NOISE_TEXTURE_HEIGHT  1024

#define INERFERENCE_TEXTURE_WIDTH   1024
#define INERFERENCE_TEXTURE_HEIGHT  128

//==============================================================
//==============================================================
void TOSD::drawNoise(float amount)
{
  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  XPLMBindTexture2d(this->noiseTextureId, 0);

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

  glColor4f(1.0f, 1.0f, 1.0f, (amount));

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
    sizeY = sizeY * (rand() / RAND_MAX + 0.3f);

    if (1.0f * rand() / RAND_MAX > (pow(amount, 0.25))) dy = 10000;

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
  if (this->osdImpl == nullptr)
    return;

  this->osdImpl->draw(this->osd_type, this->smoothed);

  if (this->videoLink != VS_NONE && g_msp.isConnected())
  {
    float amount = this->getNoiseAmount();
    this->drawNoise(amount);
    this->drawInterference(amount);
  }
}

//==============================================================
//==============================================================
int TOSD::loadTexture(const char* pFileName)
{
  char assetFileName[MAX_PATH];

  buildAssetFilename(assetFileName, pFileName);

  unsigned char* image = 0;
  unsigned width, height;

  unsigned int error = lodepng_decode32_file(&image, &width, &height, assetFileName);
  if (error)
  {
    printf("error %u: %s\n", error, lodepng_error_text(error));
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
  this->osdImpl = std::make_unique<OSD_HD>();
  this->osdImpl->setFont(this->hdFont);

  this->noiseTextureId = this->loadTexture("assets\\noise.png");
  this->interferenceTextureId = this->loadTexture("assets\\interference.png");

  this->cbConnect(CBC_DISCONNECTED);
}

//==============================================================
//==============================================================
void TOSD::destroy()
{
  if (this->osdImpl == nullptr)
    return;

  this->osdImpl->destroy();
  destroyTexture(this->noiseTextureId);
}

//==============================================================
//==============================================================
void TOSD::updateFromINAV(const TMSPSimulatorFromINAV* message)
{
  if (this->osdImpl == nullptr)
    return;

  this->osdImpl->decode(message);
}

//==============================================================
//==============================================================
void TOSD::clear()
{
  if (this->osdImpl == nullptr)
    return;

  osdImpl->clear();
}

//==============================================================
//==============================================================
void TOSD::drawString(int row, char* str)
{
  if (this->osdImpl == nullptr)
    return;

  char string[256] = { 0 };
  strncpy(string, str, 256);
  char* token = strtok(string, "\n");
  while (token != NULL) {
    int col = this->osdImpl->getColCount() / 2 - strlen(token) / 2;
    for (int i = 0; i < strlen(token); i++)
      this->osdImpl->setChar(row, col++, token[i]);

    token = strtok(NULL, "\n");
    row++;
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
    return;
  }

  this->setHDMode(true);
  this->osdImpl->disconnect();
  if (state == CBC_CONNECTION_FAILED)
  {
    g_osd.showMsg("CONNECTION FAILED");
  }
  else if (state == CBC_TIMEOUT_DISCONNECTED)
  {
    g_osd.showMsg("CONNECTION LOST");
  }
  else  if (state != CBC_CONNECTED)
  { 
    this->drawString(1, "INAV HITL DISCONNECTED");
    this->drawString(2, HITL_VERSION_STRING);
  }
}

void TOSD::setFont(std::string font)
{
  this->hdFont = font;

  if (this->osdImpl != nullptr)
    this->osdImpl->setFont(font);
}

std::string TOSD::getFont()
{
  return this->hdFont;
}

void TOSD::setHDMode(bool enable)
{
  if ((enable && this->isHD) || (!enable && !this->isHD))
    return;

  if (enable)
  {
    this->osdImpl = std::make_unique<OSD_HD>();
    this->osdImpl->setFont(this->hdFont);
    this->isHD = true;
  }
  else
  {
    this->osdImpl = std::make_unique<OSDAnalog>();
    this->osdImpl->loadFont();
    this->isHD = false;
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

  float res = d / maxD;
  float s = sin(g_simData.roll / 180.0f * 3.14f);
  res += s * s * 0.2f;
  if (res > 0.99f) res = 0.99f;

  if (res < 0.475f) res = 0.475f;
  return res;
}

//==============================================================
//==============================================================
void TOSD::showMsg(char* msg)
{
  this->clear();
  this->drawString(1, msg);
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
  if (ini[SETTINGS_SECTION].has(SETTINGS_OSD_TYPE))
  {
    this->osd_type = (TOSDType)atoi(ini[SETTINGS_SECTION][SETTINGS_OSD_TYPE].c_str());
    if (this->osd_type < OSD_NONE || this->osd_type > OSD_NTSC)
    {
      this->osd_type = OSD_AUTO;
    }
  }

  this->smoothed = ini[SETTINGS_SECTION].has(SETTINGS_OSD_SMOOTHED) && (ini[SETTINGS_SECTION][SETTINGS_OSD_SMOOTHED] == "1");

  if (ini[SETTINGS_SECTION].has(SETTINGS_VIDEOLINK_SIMULATION))
  {
    this->videoLink = (TVideoLinkSimulation)atoi(ini[SETTINGS_SECTION][SETTINGS_VIDEOLINK_SIMULATION].c_str());
    if (this->videoLink < VS_NONE || this->videoLink > VS_50KM)
    {
      this->videoLink = VS_50KM;
    }
  }

  if (ini[SETTINGS_SECTION].has(SETTINGS_HD_FONT))
  {
    this->hdFont = ini[SETTINGS_SECTION][SETTINGS_HD_FONT];
  }
  else
  {
    if (getAvaiableFonts().size() > 0)
      this->setFont(getAvaiableFonts()[0]);
  }
}

//==============================================================
//==============================================================
void TOSD::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_OSD_TYPE] = std::to_string(this->osd_type);
  ini[SETTINGS_SECTION][SETTINGS_OSD_SMOOTHED] = std::to_string(this->smoothed ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_VIDEOLINK_SIMULATION] = std::to_string(this->videoLink);
  ini[SETTINGS_SECTION][SETTINGS_HD_FONT] = this->hdFont;
}


#include "lodepng.h"

#include "osd.h"
#include "util.h"
#include "stats.h"
#include "msp.h"

TOSD g_osd;

#define FONT_IMAGE_WIDTH   209
#define FONT_IMAGE_HEIGHT  609

#define FONT_TEXTURE_WIDTH   512
#define FONT_TEXTURE_HEIGHT  1024

#define OSD_CHAR_WIDTH 12
#define OSD_CHAR_HEIGHT 18

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

//==============================================================
//==============================================================
void TOSD::drawOSD()
{
  if (this->osd_type == OSD_NONE) return;

  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  XPLMBindTexture2d(this->fontTextureId, 0);

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

  int rowsCount = OSD_ROWS;

  if (this->osd_type == OSD_NTSC)
  {
    rowsCount = OSD_ROWS_NTSC;
  }
  else if (this->osd_type == OSD_AUTO)
  {
    rowsCount = this->auto_rows;
  }

  float marginX = sx * OSD_MARGIN_HOR_PERCENT / 100.0f;
  float marginY = sy * OSD_MARGIN_VERT_PERCENT / 100.0f;

  if (rowsCount == 13)
  {
    marginY += (sy - marginY * 2) / rowsCount;
  }

  float x0 = marginX;
  float y0 = marginY;

  float cw = ( sx - marginX * 2 ) / OSD_COLS;
  float ch = ( sy - marginY * 2 ) / rowsCount;

  //a cross on 30 cols osd does not aling ok to the center of the screen
  //shift half charwidth to the left to aling crosss to screen center
  x0 -= cw / 2; 

  glBegin(GL_QUADS);

  for (int y = 0; y < rowsCount; y++)
  {
    for (int x = 0; x < OSD_COLS; x++)
    {
      //int icode = (x*y) & 511;
      //int code = MAKE_CHAR_MODE(icode, 0);

      int code = this->osdData[y * OSD_COLS + x];

      if ( CHAR_IS_BLANK( code )) continue;

      if (blink && ((MAX7456_MODE_BLINK & code) != 0) ) continue;

      int code9 = CHAR_BYTE(code) | (code & CHAR_MODE_EXT ? 0x100 : 0);
      this->drawChar(code9, x0 + x * cw, sy - (y0 + y * ch), cw, ch);
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

  if (this->videoLink != VS_NONE && g_msp.isConnected())
  {
    float amount = this->getNoiseAmount();
    this->drawNoise(amount);
    this->drawInterference(amount);
  }
}

//==============================================================
//==============================================================
void TOSD::drawChar(uint16_t code, float x1, float y1, float width, float height)
{
  int code9 = code % 0xff;

  int px = 1 + (code % 16) * (OSD_CHAR_WIDTH + 1);
  int py = 1 + (code / 16) * (OSD_CHAR_HEIGHT + 1);

  float u1 = (float)px;
  float u2 = u1 + OSD_CHAR_WIDTH;

  float v1 = (float)py;
  float v2 = v1 + OSD_CHAR_HEIGHT;

  u1 /= FONT_TEXTURE_WIDTH;
  v1 /= FONT_TEXTURE_HEIGHT;
  u2 /= FONT_TEXTURE_WIDTH;
  v2 /= FONT_TEXTURE_HEIGHT;

  float x2 = x1 + width;
  float y2 = y1 - height;

  glTexCoord2f(u1, v1);        glVertex2f(x1, y1);    
  glTexCoord2f(u2, v1);        glVertex2f(x2, y1);    
  glTexCoord2f(u2, v2);        glVertex2f(x2, y2);    
  glTexCoord2f(u1, v2);        glVertex2f(x1, y2);    
}

//==============================================================
//==============================================================
void TOSD::loadFont()
{
  char assetFileName[MAX_PATH];

  buildAssetFilename( assetFileName,  "assets\\osd_font.png");

  unsigned char* image = 0;
  unsigned width, height;

  unsigned int error = lodepng_decode32_file(&image, &width, &height, assetFileName);
  if (error)
  {
    printf("error %u: %s\n", error, lodepng_error_text(error));
    return;
  }

  if (width != FONT_IMAGE_WIDTH || height != FONT_IMAGE_HEIGHT) return;

  uint8_t* buffer = new uint8_t[FONT_TEXTURE_WIDTH * FONT_TEXTURE_HEIGHT * 4];

  memset((void*)buffer, 0, FONT_TEXTURE_WIDTH * FONT_TEXTURE_HEIGHT * 4);

  for (int cy = 0; cy < FONT_IMAGE_HEIGHT; cy++)
  {
    if ( (cy % (OSD_CHAR_HEIGHT + 1)) == 0 ) continue;

    const uint8_t* pi = image + cy * FONT_IMAGE_WIDTH * 4;
    uint8_t* pt = buffer + cy * FONT_TEXTURE_WIDTH * 4;

    for (int cx = 0; cx < FONT_IMAGE_WIDTH; cx++)
    {
      if ( ((cx % (OSD_CHAR_WIDTH + 1)) != 0) && (pi[0] != 128))
      {
        pt[0] = pt[1] = pt[2] = pi[0];
        pt[3] = 255;
      }

      pi += 4;
      pt += 4;
    }
  }

  XPLMGenerateTextureNumbers(&this->fontTextureId, 1);
  XPLMBindTexture2d(this->fontTextureId, 0);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,                   // mipmap level
    GL_RGBA,             // internal format for the GL to use.  (We could ask for a floating point tex or 16-bit tex if we were crazy!)
    FONT_TEXTURE_WIDTH,
    FONT_TEXTURE_HEIGHT,
    0,                   // border size
    GL_RGBA,             // format of color we are giving to GL
    GL_UNSIGNED_BYTE,    // encoding of our data
    buffer);

  free(image);
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
  this->loadFont();

  this->noiseTextureId = this->loadTexture("assets\\noise.png");
  this->interferenceTextureId = this->loadTexture("assets\\interference.png");

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
  this->destroyTexture(this->fontTextureId);
  this->destroyTexture(this->noiseTextureId);
}

//==============================================================
//==============================================================
void TOSD::updateFromINAV(const TMSPSimulatorFromINAV* message)
{
  if (message->osdRow == 255)
  {
    return; //no OSD data
  }

  this->auto_rows = (message->osdRow & 0x80) ? 16 : 13;

  int osdRow = message->osdRow & 0x7f;
  int osdCol = message->osdCol;

  bool highBank = false;
  bool blink = false;
  int count;

  int byteCount = 0;
  while (byteCount < (200-3-2) )
  {
    uint8_t c = message->osdRowData[byteCount++];
    if (c == 0)
    {
      c = message->osdRowData[byteCount++];
      count = (c & 0x3f);
      if (count == 0)
      {
        break; //stop
      }
      highBank ^= (c & 64)!=0;
      blink ^= (c & 128)!=0;
      c = message->osdRowData[byteCount++];
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
      this->osdData[osdRow * 30 + osdCol] = MAKE_CHAR_MODE((c | (highBank ? 0x100 : 0)), (blink ? MAX7456_MODE_BLINK : 0));
      osdCol++;
      if (osdCol == 30)
      {
        osdCol = 0;
        osdRow++;
        if (osdRow == 16)
        {
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
  memset(this->osdData, 0, OSD_ROWS*OSD_COLS * 2);
}

//==============================================================
//==============================================================
void TOSD::drawString( int row, int col, const char* str)
{

  uint16_t* p = &this->osdData[ row * OSD_COLS + col];

  while (*str && p < &this->osdData[OSD_ROWS * OSD_COLS ])
  {
    char c = *str++;  //avoid macro side effect
    *p++ = MAKE_CHAR_MODE( c, 0 );
  }

}

//==============================================================
//==============================================================
void TOSD::cbConnect(TCBConnectParm state)
{
  this->clear();

  if (state == CBC_CONNECTED)
  {
    g_simData.updateFromXPlane();
    this->home_lattitude = g_simData.lattitude;;
    this->home_longitude = g_simData.longitude;
    this->home_elevation = g_simData.elevation;
    this->roll = g_simData.roll;
  }

  if (state != CBC_CONNECTED)
  {
    this->drawString(0, 4, "INAV HITL DISCONNECTED");
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
  return res;
}

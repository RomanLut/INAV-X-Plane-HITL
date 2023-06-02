#include "osd_hd.h"
#include "util.h"
#include "stats.h"

#define SUBCMD_CLEAR_SCREEN 2
#define SUBCMD_WRITE_STRING 3
#define SUBCMD_DRAW         4

#define CHAR_WIDTH			  36
#define CHAR_HEIGHT			  54
#define CHARS_PER_FILE		256
#define BYTES_PER_PIXEL		4
#define SCREEN_COLS			  60
#define SCREEN_ROWS			  22

#define CHAR_SIZE (CHAR_HEIGHT * CHAR_WIDTH * BYTES_PER_PIXEL)
#define FONT_FILE_SIZE (CHAR_SIZE * CHARS_PER_FILE)
#define CHAR_BYTE_WIDTH (CHAR_WIDTH * BYTES_PER_PIXEL)

OSD_HD::OSD_HD()
{
}

OSD_HD::~OSD_HD()
{
  this->destroy();
}

void OSD_HD::decode(const TMSPSimulatorFromINAV* message)
{
  if (message->osdInfo.payLoadLength == 0xff)
    return;

  static int row = 0;
  static int col = 0;
  static int strIdx = 0;
  static int strLen = 0;
  static bool isExtdChar = false;

  for (int i = 0; i < message->osdInfo.payLoadLength; i++)
  {
    BYTE b = message->osdRowData[i];
    switch (state)
    {
    case OSD_STATE_SubCmd:
      if (b == SUBCMD_CLEAR_SCREEN) {
        this->clear();
      }
      else if (b == SUBCMD_WRITE_STRING) {
        row = col = strLen = strIdx = 0;
        isExtdChar = false;
        state = OSD_STATE_PositionY;
      }
      // SUBCMD_DRAW
      break;
    case OSD_STATE_PositionY:
      row = b;
      state = OSD_STATE_PositionX;
      break;
    case OSD_STATE_PositionX:
      col = b;
      state = OSD_STATE_StringInfo;
      break;
    case OSD_STATE_StringInfo:
      isExtdChar = b & 0x80;
      strLen = b & 0x7F;
      state = OSD_STATE_String;
      break;
    case OSD_STATE_String:
      if (row >= 0 && row < SCREEN_COLS && col >= 0 && col + strIdx < SCREEN_COLS)
        screen[row][col + strIdx++] = (UINT16)(isExtdChar ? b | 0x100 : b);

      if (strIdx == strLen)
        state = OSD_STATE_SubCmd;

      break;
    }
  }
  g_stats.OSDUpdates++;
  this->extractLatLon();
}

void OSD_HD::disconnect(void)
{
  state = OSD_STATE_SubCmd;
}

void OSD_HD::drawChar(int c, double left, double top, double right, double bottom, bool smoothed)
{
  XPLMBindTexture2d(this->textureIDs[c], 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);

  XPLMSetGraphicsState(0, 1, 0, 1, 1, 0, 0);
  glColor3f(1, 1, 1);

  glBegin(GL_QUADS);

  glTexCoord2f(0, 0);
  glVertex2f(left, bottom);

  glTexCoord2f(0, 1);
  glVertex2f(left, top);

  glTexCoord2f(1, 1);
  glVertex2f(right, top);

  glTexCoord2f(1, 0);
  glVertex2f(right, bottom);

  glEnd();
}

bool OSD_HD::loadFontFile(std::string path)
{
  std::vector<uint8_t> buffer(FONT_FILE_SIZE);

  std::fstream file(path, std::fstream::in | std::fstream::binary | std::fstream::ate);
  int size = 0;
  if (file.is_open()) {
    size = file.tellg();
    if (size != FONT_FILE_SIZE)
      return false;
    file.seekg(0);
    file.read((char*)buffer.data(), size);
  }
  else
    return false;

  file.close();

  for (int i = 0; i < CHARS_PER_FILE; i++) {
    // Flip character bitmap to convert to OpenGl coords (0, 0) = left down
    int charBegin = i * CHAR_SIZE;
    std::vector<uint8_t> flip(CHAR_SIZE);
    int targetIdx = CHAR_SIZE - CHAR_BYTE_WIDTH;
    for (int j = 0; j < CHAR_SIZE; j += CHAR_BYTE_WIDTH) {
      std::copy_n(buffer.begin() + charBegin + j, CHAR_BYTE_WIDTH, flip.begin() + targetIdx);
      targetIdx -= CHAR_BYTE_WIDTH;
    }

    int textureId = 0;
    XPLMGenerateTextureNumbers(&textureId, 1);
    XPLMBindTexture2d(textureId, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      CHAR_WIDTH,
      CHAR_HEIGHT,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      flip.data());
    textureIDs.push_back(textureId);
  }

  return true;
}

void OSD_HD::draw(TOSDType osd_type, bool smoothed)
{
  if (textureIDs.size() != CHARS_PER_FILE * 2)
    return;

  int screenWidth, screenHeight;
  XPLMGetScreenSize(&screenWidth, &screenHeight);

  double elementWidth = screenWidth / SCREEN_COLS;
  double elementHeight = screenHeight / SCREEN_ROWS;
  double borderLeft = (screenWidth - elementWidth * SCREEN_COLS) / 2.0f;
  double borderTop = (screenHeight - elementHeight * SCREEN_ROWS) / 2.0f;

  for (int y = 0; y < SCREEN_ROWS; y++)
  {
    for (int x = 0; x < SCREEN_COLS; x++) {

      int c = screen[y][x];

      if (c == 0 || c == 0x20)
        continue;

      double left = borderLeft + x * elementWidth;
      double top = screenHeight - (borderTop + y * elementHeight);
      double right = left + elementWidth;
      double bottom = top - elementHeight;

      drawChar(c, left, top, right, bottom, smoothed);
    }
  }
}

bool OSD_HD::loadFont(void)
{
  char dirName[MAX_PATH];
  buildAssetFilename(dirName, "assets\\fonts\\");

  if (!loadFontFile(std::string(dirName) + this->font + "\\font_inav.bin"))
  {
    LOG("Font file load error (missing of corrupted)");
    return false;
  }

  if (!loadFontFile(std::string(dirName) + this->font + "\\font_inav_2.bin"))
  {
    LOG("Font file 2 load error (missing of corrupted)");
    return false;
  }

  return true;
}

void OSD_HD::setFont(std::string fontName)
{
  this->font = fontName;
  this->destroyFontTextures();
  this->loadFont();
}

std::string OSD_HD::getFont()
{
  return this->font;
}

void OSD_HD::destroyFontTextures(void) {
  for (int id : textureIDs)
    destroyTexture(id);

  textureIDs = std::vector<int>();
}

void OSD_HD::destroy(void)
{
  this->destroyFontTextures();
}

void OSD_HD::clear(void)
{
  memset(this->screen, 0, sizeof(screen));
}

uint16_t OSD_HD::getChar(int row, int col)
{
  if (row < 0 || row > SCREEN_ROWS || col < 0 || col > SCREEN_COLS)
    return 0;

  return this->screen[row][col];
}

void OSD_HD::setChar(int row, int col, char c)
{
  if (row < 0 || row > SCREEN_ROWS || col < 0 || col > SCREEN_COLS)
    return;

  this->screen[row][col] = c;
}

int OSD_HD::getColCount()
{
  return SCREEN_COLS;
}

int OSD_HD::getRowCount()
{
  return SCREEN_ROWS;
}

#include "fontwtfos.h"

#include <fstream>

#include "util.h"

#define CHAR_WIDTH			  36
#define CHAR_HEIGHT			  54
#define CHARS_PER_FILE		  256

#define CHAR_SIZE (CHAR_HEIGHT * CHAR_WIDTH * BYTES_PER_PIXEL_RGBA)
#define FONT_FILE_SIZE (CHAR_SIZE * CHARS_PER_FILE)
#define CHAR_BYTE_WIDTH (CHAR_WIDTH * BYTES_PER_PIXEL_RGBA)


FontWtfOS::FontWtfOS(std::filesystem::path path)
{
  strcpy(this->name, ("Wtfos: " + path.filename().string()).c_str());

  std::filesystem::path fileNameBank1 = path;
  std::filesystem::path fileNameBank2 = path;
  fileNameBank1.append("font_inav.bin");
  fileNameBank2.append("font_inav_2.bin");

  std::vector<uint8_t> bank1 = this->loadBin(fileNameBank1);
  std::vector<uint8_t> bank2 = this->loadBin(fileNameBank2);

  if (bank1.empty() || bank2.empty()) {
    LOG("Unable to load font file:", this->name);
    return;
  }

  this->charWidth = CHAR_WIDTH;
  this->charHeight = CHAR_HEIGHT;

  std::vector<uint8_t>* bank;
  for (int charIndex = 0; charIndex < CHARS_PER_FILE * 2; charIndex++)
  {
    bank = charIndex >= CHARS_PER_FILE ? &bank2 : &bank1;
    // Flip character bitmap to convert to OpenGl coords (0, 0) = left down
    int charBegin = charIndex % CHARS_PER_FILE * CHAR_SIZE;
    std::vector<uint8_t> texture(CHAR_SIZE);
    int targetIdx = CHAR_SIZE - CHAR_BYTE_WIDTH;
    for (int j = 0; j < CHAR_SIZE; j += CHAR_BYTE_WIDTH) {
      std::copy_n(bank->begin() + charBegin + j, CHAR_BYTE_WIDTH, texture.begin() + targetIdx);
      targetIdx -= CHAR_BYTE_WIDTH;
    }
    this->textures.push_back(texture);
  }
}

int FontWtfOS::getCols()
{
  return 60;
}

int FontWtfOS::getRows()
{
  return 22;
}

bool FontWtfOS::isAnalog()
{
  return false;
}

std::vector<uint8_t> FontWtfOS::loadBin(std::filesystem::path path)
{
  std::vector<uint8_t> font = std::vector<uint8_t>(FONT_FILE_SIZE);

  std::fstream file(path, std::fstream::in | std::fstream::binary | std::fstream::ate);
  int size = 0;

  if (!file.is_open())
  {
    LOG("Unable to open file: %s\n", path);
    return font;
  }

  size = static_cast<int>(file.tellg());
  if (size != FONT_FILE_SIZE)
  {
    LOG("Incorrect file size: %s\n", path);
    file.close();
    return font;
  }

  file.seekg(0);
  file.read((char*)font.data(), size);

  file.close();

  return font;
}


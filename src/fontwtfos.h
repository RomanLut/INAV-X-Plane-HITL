#pragma once

#include "fontbase.h"

class FontWtfOS : public FontBase {
private:
  std::vector<uint8_t> loadBin(std::filesystem::path path);
public:
  FontWtfOS() = default;
  FontWtfOS(std::filesystem::path path);
  int getCols();
  int getRows();
  bool isAnalog();
};
#pragma once

#include "config.h"
#include "shader.h"

class OsdRenderer
{
private:

  Shader *shader;
  Shader *interferenceShader;

  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint textureArray;

  std::vector<GLuint> textures = std::vector<GLuint>();
  bool textureInitalised = false;
  void getTransform(int x, int y, int width, int height, float* transform);
  void drawCharacter(int layer, int x, int y, int width, int height);

public:
  OsdRenderer();
  ~OsdRenderer();

  int loadInterferenceTexture(const char* filename, bool smoothed);
  void loadOSDTextures(std::vector<std::vector<uint8_t>> textures, int width, int height, bool smoothed);
  void drawOSD(const uint16_t* osdData, int rows, int cols, int cellWidth, int cellHeight, int xOffset, int yOffset, bool blink);
  void drawInterferenceTexture(int textureId, int x, int y, int width, int height, float transparency);

};


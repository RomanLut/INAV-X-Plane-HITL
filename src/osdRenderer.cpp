#include "osdRenderer.h"

#include "stb/stb_image.h"
#include "osd.h"
#include "util.h"

const char* vertexShaderSource = R"(
  #version 330 core
  layout(location = 0) in vec2 aPos;
  layout(location = 1) in vec2 aTexCoord;

  out vec2 TexCoord;

  uniform mat4 transform;

  void main() 
  {
      gl_Position = transform * vec4(aPos, 0.0, 1.0);
      TexCoord = aTexCoord;
  } 
)";

const char* osdFragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;
  in vec2 TexCoord;

  uniform sampler2DArray textureArray;
  uniform int layer;

  void main()
  {
	  FragColor = texture(textureArray, vec3(TexCoord, layer));
  }
)";

const char* interferenceFragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;

  in vec2 TexCoord;

  uniform sampler2D texture1;
  uniform float transparency;

  void main()
  {
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = vec4(texColor.rgb, texColor.a * transparency);
  }
)";

int OsdRenderer::loadInterferenceTexture(const char* filename, bool smoothed)
{
  GLuint texture;
  XPLMGenerateTextureNumbers(reinterpret_cast<int*>(&texture), 1);
  XPLMBindTexture2d(texture, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);

  int width, height, channels;
  uint8_t* image = stbi_load(filename, &width, &height, &channels, 0);
  if (!image) {
    LOG("Unable to load texture");
    return -1;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  stbi_image_free(image);

  this->textures.push_back(texture);
  return  static_cast<int>(this->textures.size() - 1);
}

void OsdRenderer::loadOSDTextures(std::vector<std::vector<uint8_t>> textures, int width, int height, bool smoothed)
{
  if (this->textureInitalised) {
    glDeleteTextures(1, &textureArray);
  } else {
    this->textureInitalised = true;
  }

  XPLMGenerateTextureNumbers(reinterpret_cast<int*>(&this->textureArray), 1);
  glBindTexture(GL_TEXTURE_2D_ARRAY, this->textureArray);

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, static_cast<GLsizei>(textures.size()), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, smoothed ? GL_LINEAR : GL_NEAREST);

  for (size_t i = 0; i < textures.size(); i++) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<GLsizei>(i), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, textures[i].data());
  }

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

OsdRenderer::OsdRenderer()
{
  float vertices[] = {
        1.0f,  1.0f, 1.0f, 1.0f,  // right top
        1.0f, -1.0f, 1.0f, 0.0f,  // right bottom
       -1.0f, -1.0f, 0.0f, 0.0f,  // left bottom
       -1.0f,  1.0f, 0.0f, 1.0f   // left top
  };
  unsigned int indices[] = {
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
  };

  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->EBO);

  glBindVertexArray(this->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  this->textureArray = 0;
  this->shader = new Shader();
  this->interferenceShader = new Shader();
  this->shader->Compile(vertexShaderSource, osdFragmentShaderSource);
  this->interferenceShader->Compile(vertexShaderSource, interferenceFragmentShaderSource);
}

OsdRenderer::~OsdRenderer()
{
  delete this->shader;
  delete this->interferenceShader;
  glDeleteTextures(1, &textureArray);
  for (GLuint texture : this->textures) {
    glDeleteTextures(1, &texture);
  }
  glDeleteVertexArrays(1, &this->VAO);
  glDeleteBuffers(1, &this->VBO);
  glDeleteBuffers(1, &this->EBO);
}

void OsdRenderer::drawOSD(const uint16_t* osdData, int rows, int cols, int cellWidth, int cellHeight, int xOffset, int yOffset, bool blink)
{
  this->shader->Use();
  glBindVertexArray(this->VAO);
  glBindTexture(GL_TEXTURE_2D_ARRAY, this->textureArray);

  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
    {

      int code = osdData[y * OSD_MAX_COLS + x];

      if (CHAR_IS_BLANK(code)) continue;

      if (blink && ((MAX7456_MODE_BLINK & code) != 0)) continue;

      int code9 = CHAR_BYTE(code) | (code & CHAR_MODE_EXT ? 0x100 : 0);

      int posX = x * cellWidth + xOffset;
      int posY = y * cellHeight + yOffset;

      this->drawCharacter(code9, posX, posY, cellWidth, cellHeight);
    }
  }
  glBindVertexArray(0);
  this->shader->unUse();
}

void OsdRenderer::drawInterferenceTexture(int textureId, int x, int y, int width, int height, float transparency)
{
  this->interferenceShader->Use();
  glBindVertexArray(this->VAO);
  XPLMBindTexture2d(this->textures[textureId], 0);

  float transform[16];
  this->getTransform(x, y, width, height, transform);

  this->interferenceShader->setMatrix4fv("transform", transform);
  this->interferenceShader->setFloat("transparency", transparency);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  this->interferenceShader->unUse();
}

void OsdRenderer::getTransform(int x, int y, int width, int height, float* transform)
{
  int screenWidth, screenHeight;
  XPLMGetScreenSize(&screenWidth, &screenHeight);

  const float w = width / static_cast<float>(screenWidth);
  const float h = height / static_cast<float>(screenHeight);

  // Pixel to world coords
  float posX =  ((x + 0.5f) / screenWidth) * 2.0f - 1.0f;
  float posY =  1.0f - ((y + 0.5f) / screenHeight) * 2.0f;

  // 0,0 = Upper left
  posX += w;
  posY -= h;

  float transf[16] = {
      w,      0.0f,   0.0f,   0.0f,   // normal width
      0.0f,   h,      0.0f,   0.0f,   // normal height
      0.0f,   0.0f,   1.0f,   0.0f,   // ---
      posX,  posY,  0.0f,   1.0f    // position
  };
  memcpy(transform, transf, sizeof(float) * 16);
}

void OsdRenderer::drawCharacter(int layer, int x, int y, int width, int height)
{
  float transform[16];
  this->getTransform(x, y, width, height, transform);

  this->shader->setMatrix4fv("transform", transform);
  this->shader->setInteger("layer", layer);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}



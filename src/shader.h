#pragma once

#include "config.h"

class Shader
{
private:
  GLuint shader;

  bool checkCompileErrors(unsigned int object, std::string type);
  GLuint compileShader(GLenum type, const char* source);

public:
  Shader();
  Shader& Use();
  void unUse();
  bool Compile(const char* vertexSource, const char* fragmentSource);

  void setInteger(const char* name, int value) const;
  void setFloat(const char* name, float value) const;
  void setMatrix4fv(const char *name, const float* value) const;
};

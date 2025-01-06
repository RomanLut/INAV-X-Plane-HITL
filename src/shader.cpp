#include "shader.h"

#include "util.h"

Shader::Shader()
{
  this->shader = 0;
}

Shader& Shader::Use()
{
  glUseProgram(this->shader);
  return *this;
}

void Shader::unUse()
{
  glUseProgram(0);
}

bool Shader::Compile(const char* vertexSource, const char* fragmentSource)
{
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  if (!checkCompileErrors(vertexShader, "VERTEX")) {
    return false;
  }

  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
  if (!checkCompileErrors(fragmentShader, "FRAGMENT")) {
    return false;
  }

  this->shader = glCreateProgram();
  glAttachShader(this->shader, vertexShader);
  glAttachShader(this->shader, fragmentShader);
  glLinkProgram(this->shader);

  if (!checkCompileErrors(this->shader, "PROGRAM")) {
    return false;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return true;
}

void Shader::setInteger(const char* name, int value) const
{
  glUniform1i(glGetUniformLocation(this->shader, name), value);
}

void Shader::setFloat(const char* name, float value) const
{
  glUniform1f(glGetUniformLocation(this->shader, name), value);
}

void Shader::setMatrix4fv(const char* name, const float* value) const
{
  glUniformMatrix4fv(glGetUniformLocation(this->shader, name), 1, GL_FALSE, value);
}

GLuint Shader::compileShader(GLenum type, const char* source)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  return shader;
}

bool Shader::checkCompileErrors(unsigned int object, std::string type)
{
  int success;
  char infoLog[1024];
  if (type != "PROGRAM")
  {
    glGetShaderiv(object, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(object, 1024, NULL, infoLog);
      LOG("Unable to compile Shader");
      return false;
    }
  }
  else
  {
    glGetProgramiv(object, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(object, 1024, NULL, infoLog);
      LOG("Unable to link Shader");
      return false;
    }
  }
  return true;
}


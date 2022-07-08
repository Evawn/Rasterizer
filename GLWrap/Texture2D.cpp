//
//  Texture2D.cpp
//  Demo
//
//  Created by eschweic on 1/24/19.
//  Improved by srm Spring 2020
//

#include "Texture2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace GLWrap;

Texture2D::Texture2D(const std::string& fileName, bool srgb, bool flipVertically) {
  int force_channels = 0;
  int w, h, n;
  stbi_set_flip_vertically_on_load(flipVertically);

  std::unique_ptr<uint8_t[], void(*)(void*)> textureData(
    stbi_load(fileName.c_str(), &w, &h, &n, force_channels), 
    stbi_image_free
  );
  if (!textureData)
    throw std::invalid_argument("Could not load texture data from file " + fileName);
  glGenTextures(1, &mTextureId);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  GLint internalFormat;
  GLint format;
  switch (n) {
    case 1: internalFormat = GL_R8; format = GL_RED; break;
    case 2: internalFormat = GL_RG8; format = GL_RG; break;
    case 3: internalFormat = srgb ? GL_SRGB8 : GL_RGB8; format = GL_RGB; break;
    case 4: internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8; format = GL_RGBA; break;
    default: internalFormat = 0; format = 0; break;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
  setParameters();
}

Texture2D::Texture2D(const nanogui::Vector2i& size, GLint internalFormat, GLint format) {
  glGenTextures(1, &mTextureId);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x(), size.y(), 0, format, GL_UNSIGNED_BYTE, nullptr);
  setParameters();
}

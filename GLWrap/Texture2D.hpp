//
//  Texture2D.hpp
//  Demo
//
//  Created by eschweic on 1/24/19.
//  Improved by srm Spring 2020
//

#pragma once

#include "Util.hpp"
#include <memory>

NAMESPACE_BEGIN(GLWrap)

/// A wrapper for an OpenGL Texture2D object.
/// This class uses the RAII pattern; resources are initialized on construction
/// and deleted on destruction.
class GLWRAP_EXPORT Texture2D {
public:

  /// Create a texture from a given image.
  /// This uses stb_image to read the file data.
  /// The format is determined automatically from the input data.
  /// @throws std::runtime_error if the file cannot be read.
  Texture2D(const std::string& filename, bool srgb, bool flipVertically = true);

  /// Creates a texture with null image data.
  /// @arg size The size of the texture in pixels.
  /// @arg internalFormat The internal format of the texture. See
  ///   https://www.khronos.org/opengl/wiki/Image_Format
  /// @arg format The format of the texture. See
  ///   https://www.khronos.org/opengl/wiki/Image_Format
  Texture2D(const Eigen::Vector2i& size, GLint internalFormat = GL_RGBA8, GLint format = GL_RGBA);

  /// Wraps an existing OpenGL texture and takes ownership of it.
  Texture2D(GLint textureId) : mTextureId(textureId) { }

  /// This class tracks GPU resources and should not be copied.
  Texture2D(const Texture2D&) = delete;

  /// This class tracks GPU resources and should not be copied.
  Texture2D& operator=(const Texture2D&) = delete;

  /// Moves tracked GPU resources into this instance. The other instance is left in an invalid state
  /// but may be safely destroyed.
  Texture2D(Texture2D&& other) :
  mTextureId(other.mTextureId) {
    other.mTextureId = 0;
  }

  /// Moves tracked GPU resources into this instance. The other instance is left in an invalid state
  /// but may be safely destroyed.
  Texture2D& operator=(Texture2D&& other) {
    glDeleteTextures(1, &mTextureId);
    other.mTextureId = mTextureId;
    mTextureId = 0;
    return *this;
  }

  /// Deletes this texture.
  ~Texture2D() noexcept {
    glDeleteTextures(1, &mTextureId);
  }

  /// Return the texture ID of this instance.
  GLuint id() const { return mTextureId; }

  /// Sets the parameters of this texture. See
  /// https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glTexParameter.xml
  void setParameters(GLint textureWrapS = GL_CLAMP_TO_EDGE,
                     GLint textureWrapT = GL_CLAMP_TO_EDGE,
                     GLint textureMagFilter = GL_NEAREST,
                     GLint textureMinFilter = GL_LINEAR) const {
    if (mTextureId == 0) return;
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMinFilter);
  }

  void parameter(GLenum pname, GLint value) const {
    if (mTextureId == 0) return;
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameteri(GL_TEXTURE_2D, pname, value);
  }

  void parameter(GLenum pname, GLfloat value) const {
    if (mTextureId == 0) return;
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameterf(GL_TEXTURE_2D, pname, value);
  }

  /// Binds this texture to the speicified texture unit.
  /// @warning The specified texture unit must be in the range [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS).
  /// @arg textureUnit The index of the texture unit. For instance, `0` would bind to `GL_TEXTURE0`.
  void bindToTextureUnit(int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, id());
  }

  /// Creates a mipmap for the texture.
  void generateMipmap() const {
    glBindTexture(GL_TEXTURE_2D, id());
    glGenerateMipmap(GL_TEXTURE_2D);
  }

protected:
  /// The texture ID.
  /// This can be 0 if this instance was moved from.
  GLuint mTextureId;
};

NAMESPACE_END(GLWrap)


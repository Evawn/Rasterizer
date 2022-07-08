//
//  Framebuffer.hpp
//  cs5625
//
//  Created by eschweic on 1/24/19.
//  Improved by srm Spring 2020
//

#pragma once

#include "Texture2D.hpp"

NAMESPACE_BEGIN(GLWrap)

/// A wrapper for an OpenGL Framebuffer Object (FBO).
/// When bound, OpenGL will render to 2D textures instead of the screen.
/// This class uses the RAII pattern; resources are initialized on construction
/// and deleted on destruction.
class GLWRAP_EXPORT Framebuffer {
public:
  /// Create a framebuffer with its associated 2D textures using default formats.
  /// Each color attachment has the default Texture2D internal format (GL_RGBA8).
  /// The depth attachment, if created, has an internal format of GL_DEPTH_COMPONENT24.
  /// @arg size The size of the created textures in pixels.
  /// @arg numColorAttachments The number of color attachments bound to this FBO.
  /// @arg useDepthAttachment If true, add a depth attachment to this FBO.
  Framebuffer(const Eigen::Vector2i& size,
              int numColorAttachments = 1,
              bool useDepthAttachment = true);

  /// Create a framebuffer with only color attachments, creating new textures using 
  /// the provided formats.
  /// @throws std::runtime_error if the FBO cannot be created with the given formats.
  /// @arg size The size of each of the textures in pixels.
  /// @arg colorAttachmentFormats Pairs of internalFormat and format parameters for each color
  ///   texture.
  Framebuffer(const Eigen::Vector2i& size,
              const std::vector<std::pair<GLenum, GLenum>>& colorAttachmentFormats);

  /// Create a framebuffer with depth and color attachments, creating new textures using 
  /// the provided formats.
  /// @throws std::runtime_error if the FBO cannot be created with the given formats.
  /// @arg size The size of each of the textures in pixels.
  /// @arg colorAttachmentFormats Pairs of internalFormat and format parameters for each color
  ///   texture.
  /// @arg depthAttachmentFormats A pair of internalFormat and format parameters for the
  ///   depth texture.
  Framebuffer(const Eigen::Vector2i& size,
              const std::vector<std::pair<GLenum, GLenum>>& colorAttachmentFormats,
              const std::pair<GLenum, GLenum>& depthAttachmentFormat);

  /// Create a framebuffer that takes ownership of existing textures; this allows additional
  /// flexibility on the type and format of textures. Arguments should be transfered using 
  /// std::move().
  /// @throws std::runtime_error if the FBO cannot be created with the given textures.
  /// @arg colorAttachments The color attachments for this FBO.
  /// @arg depthAttachment The depth attachment for this FBO. This may be null.
  Framebuffer(std::vector<Texture2D> colorAttachments,
              std::unique_ptr<Texture2D> depthAttacment);

  /// This class tracks GPU resources and should not be copied.
  Framebuffer(const Framebuffer&) = delete;

  /// This class tracks GPU resources and should not be copied.
  Framebuffer& operator=(const Framebuffer&) = delete;

  /// Move tracked GPU resources into this instance. The other instance is left empty.
  Framebuffer(Framebuffer&& other);

  /// Move tracked GPU resources into this instance. Any previously owned resources are released
  /// and the other instance is left empty.
  Framebuffer& operator=(Framebuffer&& other);

  /// Delete this framebuffer and its associated textures.
  ~Framebuffer() noexcept;

  /// Bind the framebuffer object and attach all relevent textures.
  /// This must be called before modifying or drawing to the framebuffer.
  /// @arg mipmapLevel The level of the mipmap of the attachments to bind.
  ///   Note that any value besides 0 assumes that mipmap space has been allocated.
  void bind(int mipmapLevel = 0) const;

  /// Unbind the framebuffer object and unbind all relevent textures.
  void unbind() const;

  /// Returns true if the FBO is complete.
  bool complete() const;

  /// Return the framebuffer ID of this instance.
  GLuint id() const {
    return mFramebufferId;
  }

  /// Return a color texture associated with this FBO.
  /// @throws std::out_of_bounds if index is less than 0 or greater than the number of
  ///   color textures.
  const Texture2D& colorTexture(int index = 0) const;

  /// Return the depth texture associated with this FBO.
  /// @throws std::runtime_error if this FBO has no depth texture.
  const Texture2D& depthTexture() const;


protected:

  /// The framebuffer ID.
  /// This can be 0 if this instance was moved from.
  GLuint mFramebufferId;
  /// The list of color textures associated with this FBO.
  std::vector<Texture2D> mColor;
  /// The depth texture (if any) associated with this FBO.
  std::unique_ptr<Texture2D> mDepth;
};


// Implementations of inline member functions

inline
Framebuffer::Framebuffer(Framebuffer&& other)
: mFramebufferId(other.mFramebufferId),
  mColor(std::move(other.mColor)),
  mDepth(std::move(other.mDepth)) {
    other.mFramebufferId = 0;
  }

inline
Framebuffer& Framebuffer::operator=(Framebuffer&& other) {
  glDeleteFramebuffers(1, &mFramebufferId);
  mFramebufferId = other.mFramebufferId;
  mColor = std::move(other.mColor);
  mDepth = std::move(other.mDepth);
  return *this;
}

inline
Framebuffer::~Framebuffer() noexcept {
  glDeleteFramebuffers(1, &mFramebufferId);
}

inline
const Texture2D& Framebuffer::colorTexture(int index) const {
  return mColor.at(index);
}

inline
const Texture2D& Framebuffer::depthTexture() const {
  if (!mDepth) {
    throw std::runtime_error("Attempted to access a null depth buffer!");
  }
  return *mDepth;
}



NAMESPACE_END(GLWrap)


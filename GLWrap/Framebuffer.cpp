//
//  Framebuffer.cpp
//  cs5625
//
//  Created by eschweic on 1/24/19.
//  Improved by srm Spring 2020
//

#include "Framebuffer.hpp"

using namespace GLWrap;

Framebuffer::Framebuffer(const nanogui::Vector2i& size, int numColorAttachments,
                                           bool useDepthAttachment) :
mDepth(useDepthAttachment ?
       new Texture2D(size, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT) : nullptr) {
  glGenFramebuffers(1, &mFramebufferId);

  for (int colorAttachment = 0; colorAttachment < numColorAttachments; colorAttachment++) {
    mColor.emplace_back(size);
  }

  if (!complete()) throw std::runtime_error("Could not create framebuffer object!");
}

Framebuffer::Framebuffer(std::vector<Texture2D> colorAttachments,
                             std::unique_ptr<Texture2D> depthAttachment) :
mColor(std::move(colorAttachments)),
mDepth(std::move(depthAttachment)) {
  glGenFramebuffers(1, &mFramebufferId);

  if (!complete()) throw std::runtime_error("Could not create framebuffer object!");
}

Framebuffer::Framebuffer(const Eigen::Vector2i& size,
                             const std::vector<std::pair<GLenum, GLenum>>& colorAttachmentFormats) {
  glGenFramebuffers(1, &mFramebufferId);

  mColor.reserve(colorAttachmentFormats.size());
  for (const std::pair<GLenum, GLenum>& formats : colorAttachmentFormats) {
    mColor.emplace_back(size, formats.first, formats.second);
  }

  if (!complete()) throw std::runtime_error("Could not create framebuffer object!");
}

Framebuffer::Framebuffer(const Eigen::Vector2i& size,
                             const std::vector<std::pair<GLenum, GLenum>>& colorAttachmentFormats,
                             const std::pair<GLenum, GLenum>& depthAttachmentFormat) {
  glGenFramebuffers(1, &mFramebufferId);

  mColor.reserve(colorAttachmentFormats.size());
  for (const std::pair<GLenum, GLenum>& formats : colorAttachmentFormats) {
    mColor.emplace_back(size, formats.first, formats.second);
  }
  mDepth.reset(new Texture2D(size, depthAttachmentFormat.first,
                             depthAttachmentFormat.second));

  if (!complete()) throw std::runtime_error("Could not create framebuffer object!");
}


void Framebuffer::bind(int mipmapLevel) const {
  glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
  for (int colorAttachment = 0; colorAttachment < mColor.size(); colorAttachment++) {
    const Texture2D& tex = mColor[colorAttachment];
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachment,
                           GL_TEXTURE_2D, tex.id(), mipmapLevel);
  }
  if (mDepth) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepth->id(), mipmapLevel);
  }
}

void Framebuffer::unbind() const {
  for (int colorAttachment = 0; colorAttachment < mColor.size(); colorAttachment++) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachment,
                           GL_TEXTURE_2D, 0, 0);
  }
  if (mDepth) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::complete() const {
  bind();
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  bool result = status == GL_FRAMEBUFFER_COMPLETE;
  unbind();
  return result;
}


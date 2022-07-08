//
//  Util.hpp
//  cs5625
//
//  Created by eschweic on 1/26/19.
//

#pragma once

#include <iostream>
#include <nanogui/opengl.h>

#if defined(_WIN32)
#  if defined(GLWRAP_BUILD)
#    define GLWRAP_EXPORT __declspec(dllexport)
#  else
#    define GLWRAP_EXPORT __declspec(dllimport)
#  endif
#elif defined(GLWRAP_BUILD)
#  define GLWRAP_EXPORT __attribute__ ((visibility("default")))
#else
#  define GLWRAP_EXPORT
#endif

NAMESPACE_BEGIN(GLWrap)

/// Check for OpenGL error codes.
/// Calls glGetError and throws and exception or writes to stderr if an error has been triggered.
/// @throws std::runtime_error if throwOnError is true and an error is encountered.
/// @arg loc a string to indicate the location of the check, which will be included in the message.
/// @arg throwOnError If true, throws an exception when an error is encountered; otherwise,
///   prints any errors to stderr.
GLWRAP_EXPORT inline GLenum checkGLError(std::string loc = "", bool throwOnError = true) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::string errorMessage("GL ERROR");
    if (!loc.empty())
      errorMessage += " at " + loc;
    errorMessage += ": ";
    switch (error) {
      case GL_INVALID_ENUM:
        errorMessage += "Invalid Enum";
        break;
      case GL_INVALID_VALUE:
        errorMessage += "Invalid Value";
        break;
      case GL_INVALID_OPERATION:
        errorMessage += "Invalid Operation";
        break;
      case GL_OUT_OF_MEMORY:
        errorMessage += "Out Of Memory";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorMessage += "Invalid Framebuffer Operation";
        break;
      default:
        errorMessage += "Unknown Error: " + std::to_string(error);
        break;
    }
    if (throwOnError) {
      throw std::runtime_error(errorMessage);
    } else {
      std::cerr << errorMessage << std::endl;
    }
  }
  return error;
}

NAMESPACE_END(GLWrap)

// Shader.hpp

#pragma once

#include <nanogui/opengl.h>

#include "Util.hpp"

namespace GLWrap {

/*
 * A class to represent an OpenGL shader.
 *
 * This class is a thin layer over the underlying API; it owns an
 * OpenGL shader object whose lifetime matches this object.
 *
 * The primary feature of this class is the source function, which 
 * reads shader source code from a file and compiles it.
 */
class GLWRAP_EXPORT Shader {
public:

    // Create a new shader of the given type (GL_VERTEX_SHADER, etc.)
    // The name is used for error reporting.
    Shader(GLenum type, std::string name);

    // Deletes the OpenGL shader
    ~Shader();

    // Copying Shaders is not permitted because they own OpenGL resources
    Shader(const Shader &) = delete;
    Shader &operator =(const Shader &) = delete;

    // Moving Shaders transfers ownership of the OpenGL resources
    Shader(Shader &&);
    Shader &operator =(Shader &&);

    // Load source code from a file and compile.
    // Exits with an error dump if there is a compile error
    void source(const std::string &);

    // The OpenGL shader id is available for making calls 
    // that are not supported by this class.
    GLuint id() const { return shader; }

    // Name is available for debugging
    const std::string &getName() { return name; }

private:

    // We keep a name for purposes of debugging
    std::string name;

    // The OpenGL ID of the shader
    GLuint shader;
};

} // namespace
// Program.hpp

#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <nanogui/opengl.h>

#include "Util.hpp"

namespace GLWrap {

class Shader;

/*
 * A class to represent an OpenGL shader program.
 *
 * This class is a thin wrapper over the underlying API; it owns an
 * OpenGL program object whose lifetime matches this object.
 */
class GLWRAP_EXPORT Program {
public:
    
    // Create a new shader program.
    // The name is used for error reporting.
    Program(std::string name);

    // Create a shader program in one step by compiling and linking a collection of shaders.
    // Each pair gives the type and source code pathname of one shader in the program.
    // This is a convenience constructor equivalent to creating separate Shader instances,
    // attaching them to a program, and linking the program.
    Program(std::string name, std::vector<std::pair<GLenum, std::string>>);

    // Deletes the OpenGL shader program
    ~Program();

    // Copying Programs is not permitted because they own OpenGL resources
    Program(const Program &) = delete;
    Program &operator =(const Program &) = delete;

    // Moving Programs transfers ownership of the OpenGL resources
    Program(Program &&);
    Program &operator =(Program &&);

    // Attach a shader to this program
    void attach(const Shader &);

    // Detach a previously attached shader from this program
    void detach(const Shader &);

    // Link this program.  Call this after attaching all shaders needed.
    // If linking fails, prints a diagnostic message and exits.
    void link();

    // Set the value of the uniform variable in this program with the given name.
    // The OpenGL type passed to the uniform is determined by overloading among
    // this set of functions.  If the variable does not exist or is inactive,
    // a warning is printed and nothing else happens.
    void uniform(const std::string &name, int);
    void uniform(const std::string &name, float);
    void uniform(const std::string &name, const Eigen::Vector2f &);  // GLSL type vec2
    void uniform(const std::string &name, const Eigen::Vector3f &);  // GLSL type vec3
    void uniform(const std::string &name, const Eigen::Vector4f &);  // GLSL type vec4
    void uniform(const std::string &name, const Eigen::Matrix2f &);  // GLSL type mat2
    void uniform(const std::string &name, const Eigen::Matrix3f &);  // GLSL type mat3
    void uniform(const std::string &name, const Eigen::Matrix4f &);  // GLSL type mat4

    // Find the location in the linked program of a uniform by name
    // -1 means there is no active uniform with that name
    int getUniformLocation(const std::string &name);

    // Find the location in the linked program of an attribute by name
    // -1 means there is no active attribute with that name
    int getAttribLocation(const std::string &name);

    // Make this program the active program.
    void use();

    // Make no program active.
    static void unuse();

    // The OpenGL program id is available for making calls
    // that are not supported by this class.
    GLuint id() const { return program; }

    const std::string &getName() { return name; }

private:

    std::string name;
    GLuint program;
    std::vector<Shader> ownedShaders;
};

} // namespace
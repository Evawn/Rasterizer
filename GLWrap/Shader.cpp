// Shader.cpp
#include <iostream>
#include <fstream>
#include <sstream>

#include "Shader.hpp"

using namespace GLWrap;


// Creating and destroying a Shader creates and deletes the OpenGL shader
Shader::Shader(GLenum type, std::string name) 
    : name(name) {
    shader = glCreateShader(type);
}

Shader::~Shader() {
    glDeleteShader(shader);
}

// Move-constructing a shader leaves the other Shader empty
Shader::Shader(Shader &&other) {
    shader = other.shader;
    other.shader = 0;
}

// Move-assigning a shader deletes any owned shader and leaves the other Shader empty
Shader &Shader::operator =(Shader &&other) {
    glDeleteShader(shader);
    shader = other.shader;
    other.shader = 0;
    return *this;  
}

// Load source code and compile
// on a compile error, prints a report and exits
void Shader::source(const std::string &filename) {

    // Read contents of source file into a string
    std::ifstream ifs(filename);
    if (ifs.fail()) {
        std::cerr << "Could not open source file for shader '" << name << std::endl;
        std::exit(1);
    }

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string shaderText = ss.str();

    // Provide contents of string to GLSL compiler
    const char *source_p = shaderText.c_str();
    glShaderSource(shader, 1, &source_p, nullptr);
    glCompileShader(shader);

    // Check for errors and write diagnostics if there are any
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char infoLog[4096];
        glGetShaderInfoLog(shader, 4096, nullptr, infoLog);
        std::cerr << "Shader compilation failed for shader '" << name << "':" << std::endl;
        std::cerr << infoLog << std::endl;
        std::exit(1);
    }
}


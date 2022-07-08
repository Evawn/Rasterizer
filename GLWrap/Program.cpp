#include <iostream>

#include "Program.hpp"
#include "Shader.hpp"

using namespace GLWrap;


Program::Program(std::string name) 
: name(name) {
    program = glCreateProgram();
}

Program::Program(std::string name, std::vector<std::pair<GLenum, std::string>> specs) 
: name(name) {
    program = glCreateProgram();

    for (const auto &p : specs) {
        GLenum shaderType = p.first;
        std::string filename = p.second;
        Shader s = Shader(shaderType, filename);
        s.source(filename);
        attach(s);
        ownedShaders.emplace_back(std::move(s));
    }

    link();
}

Program::~Program() {
    glDeleteProgram(program);
}

// Move-constructing a Program leaves the other Program empty
Program::Program(Program &&other) {

    program = other.program;
    other.program = 0;
}

// Move-assigning a Program deletes any owned program and shaders
// and leaves the other Program empty
Program &Program::operator =(Program &&other) {
    glDeleteProgram(program);
    program = other.program;
    other.program = 0;
    ownedShaders = std::move(other.ownedShaders);
    return *this;
}

void Program::attach(const Shader &s) {
    glAttachShader(program, s.id());
}

void Program::detach(const Shader &s) {
    glDetachShader(program, s.id());
}

void Program::link() {
    glLinkProgram(program);
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char infoLog[4096];
        glGetProgramInfoLog(program, 4096, nullptr, infoLog);
        std::cerr << "Linking failed for program '" << name << "':" << std::endl;
        std::cerr << infoLog << std::endl;
        std::exit(1);
    }    
}

static int getUniformLocationWithWarning(GLuint program, const std::string &progName, const std::string &varName) {
    int loc = glGetUniformLocation(program, varName.c_str());
    if (loc == -1) {
        std::cerr << "Warning: '" << varName 
            << "' is not an active uniform in program '" << progName 
            << "'." << std::endl;
    }
    return loc;
}

void Program::uniform(const std::string &varName, int i) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniform1i(loc, i);
    }
}

void Program::uniform(const std::string &varName, float f) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniform1f(loc, f);
    }
}

void Program::uniform(const std::string &varName, const Eigen::Vector2f &v) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniform2fv(loc, 1, v.data());
    }
}

void Program::uniform(const std::string &varName, const Eigen::Vector3f &v) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniform3fv(loc, 1, v.data());
    }
}

void Program::uniform(const std::string &varName, const Eigen::Vector4f &v) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniform4fv(loc, 1, v.data());
    }
}

void Program::uniform(const std::string &varName, const Eigen::Matrix2f &m) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniformMatrix2fv(loc, 1, false, m.data());
    }
}

void Program::uniform(const std::string &varName, const Eigen::Matrix3f &m) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniformMatrix3fv(loc, 1, false, m.data());
    }
}

void Program::uniform(const std::string &varName, const Eigen::Matrix4f &m) {
    int loc = getUniformLocationWithWarning(program, name, varName);
    if (loc != -1) {
        glUseProgram(program);
        glUniformMatrix4fv(loc, 1, false, m.data());
    }
}

int Program::getAttribLocation(const std::string &name) {
    return glGetAttribLocation(program, name.c_str());
}

int Program::getUniformLocation(const std::string &name) {
    return glGetUniformLocation(program, name.c_str());
}


void Program::use() {
    glUseProgram(program);
}

void Program::unuse() {
    glUseProgram(0);
}

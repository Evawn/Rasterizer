// Mesh.cpp

#include <utility>

#include "Mesh.hpp"

#include "Util.hpp"

using namespace GLWrap;


Mesh::Mesh() {
    // Create a VAO in OpenGL
    glGenVertexArrays(1, &vao);
    // indexBuffer is zero
    // vertexBuffers is empty
}

Mesh::~Mesh() {
    // Delete the VAO and any buffers owned by this mesh
    if (vao) glDeleteVertexArrays(1, &vao);
    if (indexBuffer) glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(vertexBuffers.size(), vertexBuffers.data());
}



// Move-constructing a mesh leaves the source mesh empty
Mesh::Mesh(Mesh &&other) noexcept :
    vertexBuffers(std::move(other.vertexBuffers)) {
    vao = other.vao;
    other.vao = 0;
    indexBuffer = other.indexBuffer;
    other.indexBuffer = 0;
    indexMode = other.indexMode;
    other.indexMode = 0;
    indexLength = other.indexLength;
    other.indexLength = 0;
}

// Move-assigning a mesh leaves the source mesh empty
// and releases any VAO or buffers previously owned by this mesh
Mesh &Mesh::operator=(Mesh &&other) {
    if (&other == this) return *this;

    if (vao) glDeleteVertexArrays(1, &vao);
    if (indexBuffer) glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(vertexBuffers.size(), vertexBuffers.data());

    vao = other.vao;
    other.vao = 0;
    indexBuffer = other.indexBuffer;
    other.indexBuffer = 0;
    indexMode = other.indexMode;
    other.indexMode = 0;
    indexLength = other.indexLength;
    other.indexLength = 0;

    vertexBuffers = std::move(other.vertexBuffers);

    return *this;
}


template <class T, int N>
void Mesh::_setAttribute(int index, Eigen::Matrix<T, N, Eigen::Dynamic> data) {

    // Make space for the new buffer in our list, deleting anything that used to be there
    if (vertexBuffers.size() <= index)
        vertexBuffers.resize(index + 1);
    if (vertexBuffers[index])
        glDeleteBuffers(1, &vertexBuffers[index]);
    GLuint &buf = vertexBuffers[index];

    // Create a vertex array buffer and copy the data into it
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, N * sizeof(T) * data.cols(), 
                 (const void *) data.data(), GL_STATIC_DRAW);

    // Attach the buffer to our VAO at the desired index and enable it
    glBindVertexArray(vao);
    glVertexAttribPointer(index, N, std::is_same<T, float>::value ? GL_FLOAT : GL_INT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(index);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    checkGLError("Mesh::_setAttribute end");
}

void Mesh::setAttribute(int index, Eigen::Matrix<float, 1, Eigen::Dynamic> data) {
    _setAttribute(index, data);
}

void Mesh::setAttribute(int index, Eigen::Matrix<float, 2, Eigen::Dynamic> data) {
    _setAttribute(index, data);
}

void Mesh::setAttribute(int index, Eigen::Matrix<float, 3, Eigen::Dynamic> data) {
    _setAttribute(index, data);
}

void Mesh::setAttribute(int index, Eigen::Matrix<float, 4, Eigen::Dynamic> data) {
    _setAttribute(index, data);
}

void Mesh::setAttribute(int index, Eigen::Matrix<int, 4, Eigen::Dynamic> data) {
    _setAttribute(index, data);
}


void Mesh::setIndices(Eigen::VectorXi data, GLenum mode) {

    if (indexBuffer)
        glDeleteBuffers(1, &indexBuffer);

    // Create an index buffer, attach it to the VAO, and copy the data into it
    glGenBuffers(1, &indexBuffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(int),
                 (const void *) data.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    // Remember the info that will be needed to draw this
    indexMode = mode;
    indexLength = data.size();

    checkGLError("Mesh::setIndices");
}


void Mesh::drawElements() const {

    // Bind the VAO and draw
    glBindVertexArray(vao);
    glDrawElements(indexMode, indexLength, GL_UNSIGNED_INT, (void *) 0);
    glBindVertexArray(0);

    checkGLError("Mesh::drawElements end");
}


void Mesh::drawArrays(GLenum mode, int first, int count) const {

    // Bind the VAO and draw
    glBindVertexArray(vao);
    glDrawArrays(mode, first, count);
    glBindVertexArray(0);

    checkGLError("Mesh::drawArrays end");
}


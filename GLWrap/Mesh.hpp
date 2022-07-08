// Mesh.hpp

#pragma once

#include <nanogui/opengl.h>

#include "Util.hpp"

namespace GLWrap {

/*
 * A class to represent an Mesh stored in OpenGL.
 *
 * There is a bit too much flexibility in OpenGL to provide a simple 
 * wrapper that is also general purpose, so this class sacrifices a
 * bit of flexibility in favor of simplicity.
 * 
 * A Mesh represents two kinds of OpenGL resources:
 *   1. An OpenGL Vertex Array Object (VAO) and 
 *   2. A set of buffers bound to that VAO, containing attribute and index data.
 *
 * The setup is deliberately limited in some ways:
 *  * 32-bit floats and 32-bit ints are the only supported datatypes
 *  * only scalar and vec[234] attribute types are supported
 *  * indexed meshes are always drawn in full
 *  * each attribute comes contiguously from a separate buffer
 *  * buffers are owned by meshes and will not be shared between them
 *
 * This class does not keep track of names for attributes.  Instead
 * each attribute array is bound to a fixed index; the expectation is
 * that the GLSL code that consumes these attributes will use the
 * `layout (location = <index>)` specifier to indicate which attribute
 * matches with which array.
 */
class GLWRAP_EXPORT Mesh {
public:

    // A newly created mesh owns its VAO but no buffers
    Mesh();

    // Deleting a mesh releases all GPU resources it owns
    ~Mesh();

    // Copying is not allowed because a Mesh owns GPU resources
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    // Moving is allowed, and transfers ownership of the GPU resources
    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other);

    // Provide values for the vertex attribute at a particular index:
    //  1. Create an OpenGL buffer owned by this mesh
    //  2. Upload the provided data to that buffer
    //  3. Configure the VAO with an enabled attribute array at the
    //     given index that reads from this buffer.
    // Any buffer previously bound at this index is deleted.
    // The dimension of the vector type of the attribute is determined by
    // the argument type.
    void setAttribute(int index, Eigen::Matrix<float, 1, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 2, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 3, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 4, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<int, 4, Eigen::Dynamic> data);

    // Provide indices that define primitives, 
    // and the drawing mode (GL_TRIANGLES, etc.) that will be used by drawElements.
    void setIndices(Eigen::VectorXi data, GLenum mode);

    // Draw the entire mesh using glDrawElements (using index buffer)
    void drawElements() const;

    // Draw the mesh using glDrawArrays (using just the attribute buffers)
    void drawArrays(GLuint mode, int first, int count) const;

private:

    // Template to simplify writing the various setAttribute functions
    template<class T, int N>
    void _setAttribute(int index, Eigen::Matrix<T, N, Eigen::Dynamic> data);

    // OpenGL identifiers for the owned resources
    GLuint vao;
    GLuint indexBuffer;
    std::vector<GLuint> vertexBuffers;

    // Mode and length of index buffer
    GLenum indexMode;
    GLuint indexLength;
};

} // namespace


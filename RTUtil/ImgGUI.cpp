//
//  ImgGUI.cpp
//  Demo
//
//  Created by eschweic on 1/23/19.
//

#include "ImgGUI.hpp"
#include <nanogui/window.h>
#include <nanogui/glcanvas.h>

#include <cpplocate/cpplocate.h>

#include "GLWrap/Util.hpp"
#include "GLWrap/Program.hpp"
#include "GLWrap/Texture2D.hpp"
#include "GLWrap/Mesh.hpp"

namespace RTUtil {

ImgGUI::ImgGUI(int width, int height) :
windowWidth(width),
windowHeight(height),
nanogui::Screen(Eigen::Vector2i(width, height), "NanoGUI Demo", false) {

  // Look up paths to shader source code
  const std::string resourcePath =
    cpplocate::locatePath("resources/Common", "", nullptr) + "resources/";
  const std::string fsqVertSrcPath = resourcePath + "Common/shaders/fsq.vert";
  const std::string srgbFragSrcPath = resourcePath + "Common/shaders/srgb.frag";

  // Compile shader program
  try {
    srgbShader.reset(new GLWrap::Program("srgb", {
      { GL_VERTEX_SHADER, fsqVertSrcPath }, 
      { GL_FRAGMENT_SHADER, srgbFragSrcPath }
    }));
  } catch (std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    exit(1);
  }

  // Upload a two-triangle mesh for drawing a full screen quad
  Eigen::MatrixXf vertices(5, 4);
  vertices.col(0) << -1.0f, -1.0f, 0.0f, 0.0f, 0.0f;
  vertices.col(1) << 1.0f, -1.0f, 0.0f, 1.0f, 0.0f;
  vertices.col(2) << 1.0f, 1.0f, 0.0f, 1.0f, 1.0f;
  vertices.col(3) << -1.0f, 1.0f, 0.0f, 0.0f, 1.0f;

  Eigen::Matrix<float, 3, Eigen::Dynamic> positions = vertices.topRows<3>();
  Eigen::Matrix<float, 2, Eigen::Dynamic> texCoords = vertices.bottomRows<2>();

  fsqMesh.reset(new GLWrap::Mesh());

  fsqMesh->setAttribute(srgbShader->getAttribLocation("vert_position"), positions);
  fsqMesh->setAttribute(srgbShader->getAttribLocation("vert_texCoord"), texCoords);

  // Allocate texture
  imgTex.reset(new GLWrap::Texture2D(
    Eigen::Vector2i(windowWidth, windowHeight), GL_RGBA32F));

  // Allocate image for display
  img_data = Eigen::VectorXf::Zero(windowWidth*windowHeight*3);

  // Arrange windows in the layout we have described
  performLayout();
  // Make the window visible and start the application's main loop
  setVisible(true);
}



ImgGUI::~ImgGUI() {
}


bool ImgGUI::keyboardEvent(int key, int scancode, int action, int modifiers) {

  // Parent gets first chance to handle event.
  if (Screen::keyboardEvent(key, scancode, action, modifiers))
    return true;

  // If the user presses the escape key...
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // ...exit the application.
    setVisible(false);
    return true;
  }

  // Up and down arrows to adjust exposure
  if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
    exposure *= 2.0;
    return true;
  }
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
    exposure /= 2.0;
    return true;
  }

  // Otherwise, the event is not handled here.
  return false;
}


void ImgGUI::drawContents() {

  // Clear (hardly necessary but makes it easier to recognize viewport issues)
  glClearColor(0.0, 0.2, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Call subclass to update the displayed image
  computeImage();

  // Copy image data to OpenGL
  glBindTexture(GL_TEXTURE_2D, imgTex->id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, img_data.data()); 

  // Set up shader to convert to sRGB and write to default framebuffer
  srgbShader->use();
  imgTex->bindToTextureUnit(0);
  srgbShader->uniform("image", 0);
  srgbShader->uniform("exposure", exposure);

  // Set viewport
  Eigen::Vector2i framebufferSize;
  glfwGetFramebufferSize(glfwWindow(), &framebufferSize.x(), &framebufferSize.y());
  glViewport(0, 0, framebufferSize.x(), framebufferSize.y());

  // Draw the full screen quad
  fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

  GLWrap::checkGLError();

}

}

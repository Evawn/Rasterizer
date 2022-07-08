/*
 * Cornell CS5625
 * RTUtil library
 *
 * A class that simplifies setting up a Nanogui application that displays a high dynamic
 * range image.
 *
 * Author: srm, Spring 2020
 */


#ifndef ImgGUI_hpp
#define ImgGUI_hpp

#include <nanogui/screen.h>

#include "common.hpp"

// Forward Declarations

namespace GLWrap {
  class Texture2D;
  class Program;
  class Mesh;
}


namespace RTUtil {

  /*
   * ImgGUI
   *
   * This is a base class for simple Nanogui-based apps that want to display an image that
   * is computed on the CPU.  To make such an app, a derived class just has to override the
   * computeImage() member function with code to update the floating-point image stored in
   * img_data.  It will be converted to sRGB in the process of displaying it.
   *
   * This class provides a simple keyboard-based UI for adjusting exposure (using the up
   * and down arrow keys), and provides for adding additional event handling by overriding
   * the event handlers originally defined in nanogui::Widget.
   */

  class RTUTIL_EXPORT ImgGUI : public nanogui::Screen {
  public:

    /// Constructor that sets the fixed size of the window.
    ImgGUI(int width, int height);

    virtual ~ImgGUI();

    /// Keyboard event handler; provides quit-on-ESC, exposure adjustment, and can be 
    /// overridden to add additional behavior.
    /// @arg key The GLFW key value of the event (GLFW_KEY_A, etc.).
    /// @arg scancode The scancode of the event; unique for every key.
    /// @arg action The GLFW action value of the event (GLFW_PRESS, etc.).
    /// @arg modifiers GLFW modifier flags for any active modifiers (GLFW_MOD_SHIFT, etc.).
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    /// Draws the UI and all child windows with the given context.
    virtual void draw(NVGcontext *ctx) override {
      Screen::draw(ctx);
    }

    /// Draws the background of this window.
    /// This is where all OpenGL calls for this window happen.
    virtual void drawContents() override;

    /// Compute the image to be displayed.  Derived classes must override this to provide
    /// content.  The job of this method is to update the pixel values in @c img_data.
    virtual void computeImage() = 0;


  protected:

    /// The width and height of the application window in nominal pixels.
    /// These are also the dimensions of the displayed image.
    /// They may differ from the actual screen resolution on high dpi displays.
    int windowWidth, windowHeight;

    // Storage for displayed floating-point image in a flat
    // row-major array.  Pixel (i,j), channel k is at
    // img_data[3 * (windowWidth * iy + ix) + k]
    Eigen::VectorXf img_data;

    // Exposure for HDR -> LDR conversion
    float exposure = 1.0f;


  private:

    // Simple shader for writing out image data
    std::unique_ptr<GLWrap::Program> srgbShader;

    // Texture to hold rendered image
    std::unique_ptr<GLWrap::Texture2D> imgTex;

    // Mesh for full screen quad
    std::unique_ptr<GLWrap::Mesh> fsqMesh;

  };

}

#endif /* ImgGUI_hpp */

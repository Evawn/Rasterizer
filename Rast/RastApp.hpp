//
//  TetraApp.hpp
//  Demo of basic usage of nanogui and GLWrap to get a simple object on the screen.
//
//  Created by srm, March 2020
//

#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Mesh.hpp>
#include <GLWrap/Framebuffer.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <RTUtil/microfacet.hpp>
#include <RTUtil/Sky.hpp>

#include <generator.hpp>

using namespace std;

class RastApp : public nanogui::Screen
{
public:
  RastApp(string fileName);

  virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
  virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
  virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;
  virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) override;

  virtual void drawContents() override;

private:
  int loop = 0;
  void traverseDrawForward(shared_ptr<Node> node, Eigen::Affine3f acc);
  void findLightsForward(shared_ptr<Node> node, Eigen::Affine3f acc);

  void traverseDrawDeferred(shared_ptr<Node> node, Eigen::Affine3f acc);
  void accumulateLightsDeferred(shared_ptr<Node> node, Eigen::Affine3f acc);
  void traverseDrawLight(shared_ptr<Node> node, Eigen::Affine3f acc);

  static const int windowWidth;
  static const int windowHeight;

  std::unique_ptr<GLWrap::Program> forwardShader;

  std::unique_ptr<GLWrap::Program> geometryPass;
  std::unique_ptr<GLWrap::Program> shadowPass;
  std::unique_ptr<GLWrap::Program> shadingPass;
  std::unique_ptr<GLWrap::Program> ambientPass;
  std::unique_ptr<GLWrap::Program> blurPass;
  std::unique_ptr<GLWrap::Program> mergePass;
  std::unique_ptr<GLWrap::Program> postProcessingPass;

  std::unique_ptr<GLWrap::Framebuffer> geometryBuffer;
  std::unique_ptr<GLWrap::Framebuffer> accumulationBuffer;
  std::unique_ptr<GLWrap::Framebuffer> shadowBuffer;
  std::unique_ptr<GLWrap::Framebuffer> mergeBuffer;
  std::unique_ptr<GLWrap::Framebuffer> blurBuffer;

  std::shared_ptr<Scene> scene;

  std::shared_ptr<RTUtil::PerspectiveCamera> cam;
  std::unique_ptr<RTUtil::DefaultCC> cc;

  nanogui::Color backgroundColor;

  std::unique_ptr<GLWrap::Mesh> fsqMesh;

  RTUtil::Sky sky;

  int ks[4] = {1, 3, 5, 7};
  float sigmas[4] = {6.2, 24.9, 81.0, 263.0};
  const GLWrap::Texture2D *blurs[4];

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

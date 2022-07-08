//
//  TetraApp.cpp
//
//  Created by srm, March 2020
//

#include "RastApp.hpp"
#include <nanogui/window.h>
#include <nanogui/glcanvas.h>
#include <nanogui/layout.h>

#include <cpplocate/cpplocate.h>

// Fixed screen size is awfully convenient, but you can also
// call Screen::setSize to set the size after the Screen base
// class is constructed.
const int RastApp::windowWidth = 800;
const int RastApp::windowHeight = 600;

bool useDeferred = true;
int bufferNum = 0;

// Constructor runs after nanogui is initialized and the OpenGL context is current.
RastApp::RastApp(string fileName)
    : nanogui::Screen(Eigen::Vector2i(windowWidth, windowHeight), "Our Rasterizer", false),
      backgroundColor(0.0f, 0.0f, 0.0f, 0.0f),
      sky(M_PI / 18.0, 2)
{

  const std::string resourcePath =
      cpplocate::locatePath("resources/Common", "", nullptr) + "resources/";

  // Set up a simple shader program by passing the shader filenames to the convenience constructor
  forwardShader.reset(new GLWrap::Program("Forward Shader", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/vert.vs"},
                                                             {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/frag.fs"}}));

  geometryPass.reset(new GLWrap::Program("Geometry Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/vert.vs"},
                                                           {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/bind.fs"}}));

  shadowPass.reset(new GLWrap::Program("Shadow Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/vert.vs"},
                                                       {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/trivial.fs"}}));

  shadingPass.reset(new GLWrap::Program("Shading Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/fsq.vert"},
                                                         {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/shade.fs"}}));

  ambientPass.reset(new GLWrap::Program("Ambient Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/fsq.vert"},
                                                         {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/sunsky.fs"},
                                                         {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/amb.fs"}}));

  blurPass.reset(new GLWrap::Program("Blur Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/fsq.vert"},
                                                   {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/blur.fs"}}));

  mergePass.reset(new GLWrap::Program("Merge Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/fsq.vert"},
                                                     {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/merge.fs"}}));

  postProcessingPass.reset(new GLWrap::Program("Post Processing Pass", {{GL_VERTEX_SHADER, resourcePath + "../Rast/shaders/fsq.vert"},
                                                                        {GL_FRAGMENT_SHADER, resourcePath + "../Rast/shaders/srgb.frag"}}));

  this->scene = Generator::generateScene(fileName);

  // Create a camera in a default position, respecting the aspect ratio of the window.
  cam = make_shared<RTUtil::PerspectiveCamera>(scene->cam);
  cc.reset(new RTUtil::DefaultCC(cam));
  float as = this->scene->cam.getAspectRatio();
  Screen::setSize(Eigen::Vector2i(windowWidth, windowWidth / as));

  // Set viewport
  Eigen::Vector2i framebufferSize;
  glfwGetFramebufferSize(glfwWindow(), &framebufferSize.x(), &framebufferSize.y());
  glViewport(0, 0, framebufferSize.x(), framebufferSize.y());

  // backgroundColor = nanogui::Color(scene->bgColor.x(), scene->bgColor.y(), scene->bgColor.z(), 0.0);

  // MAKE BUFFER WITH 3 COLOR ATTACHMENTS
  geometryBuffer.reset(new GLWrap::Framebuffer(framebufferSize, 3, true));

  // BUFFER FOR SHADING PASS
  vector<pair<GLenum, GLenum> > colorAttachmentFormat = vector<pair<GLenum, GLenum> >();
  colorAttachmentFormat.push_back(make_pair(GL_RGBA32F, GL_RGBA));
  accumulationBuffer.reset(new GLWrap::Framebuffer(framebufferSize, colorAttachmentFormat));

  // SHADOW BUFFER
  // shadowBuffer.reset(new GLWrap::Framebuffer(Eigen::Vector2i(1024, 1024), 0, true));
  shadowBuffer.reset(new GLWrap::Framebuffer(framebufferSize, 0, true));

  // BLUR BUFFER
  vector<pair<GLenum, GLenum> > colorAttachmentFormatBlur = vector<pair<GLenum, GLenum> >();
  colorAttachmentFormatBlur.push_back(make_pair(GL_RGBA32F, GL_RGBA));
  colorAttachmentFormatBlur.push_back(make_pair(GL_RGBA32F, GL_RGBA));
  blurBuffer.reset(new GLWrap::Framebuffer(framebufferSize, colorAttachmentFormatBlur));

  vector<pair<GLenum, GLenum> > colorAttachmentFormatMerge = vector<pair<GLenum, GLenum> >();
  colorAttachmentFormatMerge.push_back(make_pair(GL_RGBA32F, GL_RGBA));
  mergeBuffer.reset(new GLWrap::Framebuffer(framebufferSize, colorAttachmentFormatMerge));

  // Upload a two-triangle mesh for drawing a full screen quad
  Eigen::MatrixXf vertices(5, 4);
  vertices.col(0) << -1.0f, -1.0f, 0.0f, 0.0f, 0.0f;
  vertices.col(1) << 1.0f, -1.0f, 0.0f, 1.0f, 0.0f;
  vertices.col(2) << 1.0f, 1.0f, 0.0f, 1.0f, 1.0f;
  vertices.col(3) << -1.0f, 1.0f, 0.0f, 0.0f, 1.0f;

  Eigen::Matrix<float, 3, Eigen::Dynamic> positions = vertices.topRows<3>();
  Eigen::Matrix<float, 2, Eigen::Dynamic> texCoords = vertices.bottomRows<2>();

  fsqMesh.reset(new GLWrap::Mesh());
  fsqMesh->setAttribute(postProcessingPass->getAttribLocation("vert_position"), positions);
  fsqMesh->setAttribute(postProcessingPass->getAttribLocation("vert_texCoord"), texCoords);

  // NanoGUI boilerplate
  performLayout();
  setVisible(true);
}

bool RastApp::keyboardEvent(int key, int scancode, int action, int modifiers)
{
  if (Screen::keyboardEvent(key, scancode, action, modifiers))
    return true;

  if (action == GLFW_PRESS)
  {
    switch (key)
    {
    case GLFW_KEY_SPACE:
      useDeferred = !useDeferred;
      if (useDeferred)
      {
        cout << "Using the deferred shader!\n";
      }
      else
      {
        cout << "Using only forward shading!\n";
      }
      return true;
    case GLFW_KEY_ESCAPE:
      setVisible(false);
      return true;
    case GLFW_KEY_1:
      bufferNum = 0;
      return true;
    case GLFW_KEY_2:
      bufferNum = 1;
      return true;
    case GLFW_KEY_3:
      bufferNum = 2;
      return true;
    default:
      return true;
    }
  }
  return cc->keyboardEvent(key, scancode, action, modifiers);
}

bool RastApp::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
  return Screen::mouseButtonEvent(p, button, down, modifiers) ||
         cc->mouseButtonEvent(p, button, down, modifiers);
}

bool RastApp::mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
  return Screen::mouseMotionEvent(p, rel, button, modifiers) ||
         cc->mouseMotionEvent(p, rel, button, modifiers);
}

bool RastApp::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel)
{
  return Screen::scrollEvent(p, rel) ||
         cc->scrollEvent(p, rel);
}

void RastApp::findLightsForward(shared_ptr<Node> node, Eigen::Affine3f acc)
{
  acc = acc * node->transform;

  for (int i = 0; i < node->lightInfos.size(); i++)
  {
    if (node->lightInfos[i]->type == RTUtil::Point)
    {
      Eigen::Vector3f power = node->lightInfos[i]->power;
      Eigen::Vector3f pos = acc * node->lightInfos[i]->position;
      forwardShader->uniform("lightPos", pos);
      forwardShader->uniform("power", power);
      return;
    }
  }

  for (int i = 0; i < node->children.size(); i++)
  {
    findLightsForward(node->children[i], acc);
  }
}

void RastApp::traverseDrawForward(shared_ptr<Node> node, Eigen::Affine3f acc)
{
  acc = acc * node->transform;
  forwardShader->uniform("mM", acc.matrix());

  for (int i = 0; i < node->meshInfos.size(); i++)
  {
    shared_ptr<nori::BSDF> mat = node->meshInfos[i]->material;
    shared_ptr<nori::Microfacet> micro = dynamic_pointer_cast<nori::Microfacet>(mat);

    float alpha = micro->alpha();
    float eta = micro->eta();
    Eigen::Vector3f kd = micro->diffuseReflectance();
    forwardShader->uniform("alpha", alpha);
    forwardShader->uniform("eta", eta);
    forwardShader->uniform("k_d", kd);

    node->meshInfos[i]->mesh->drawElements();
  }

  for (int i = 0; i < node->children.size(); i++)
  {
    traverseDrawForward(node->children[i], acc);
  }
}

void RastApp::traverseDrawLight(shared_ptr<Node> node, Eigen::Affine3f acc)
{
  acc = acc * node->transform;
  shadowPass->uniform("mM", acc.matrix());

  for (int i = 0; i < node->meshInfos.size(); i++)
  {
    node->meshInfos[i]->mesh->drawElements();
  }

  for (int i = 0; i < node->children.size(); i++)
  {
    traverseDrawLight(node->children[i], acc);
  }
}

void RastApp::accumulateLightsDeferred(shared_ptr<Node> node, Eigen::Affine3f acc)
{
  acc = acc * node->transform;

  for (int i = 0; i < node->lightInfos.size(); i++)
  {
    if (node->lightInfos[i]->type == RTUtil::Point)
    {
      Eigen::Vector3f power = node->lightInfos[i]->power;
      Eigen::Vector3f pos = acc * node->lightInfos[i]->position;
      shadingPass->uniform("lightPos", pos);
      shadingPass->uniform("power", power);

      // // PERFORM SHADOW PASS
      RTUtil::PerspectiveCamera lightCam = RTUtil::PerspectiveCamera(
          pos, // eye
          Eigen::Vector3f(0, 0, 0),
          Eigen::Vector3f(0, 1, 0), // up
          1,                        // aspect
          1,
          50, // near, far
          1   // fov
      );
      shadowPass->uniform("mV", lightCam.getViewMatrix().matrix());
      shadowPass->uniform("mP", lightCam.getProjectionMatrix().matrix());
      shadowBuffer->bind(0);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      shadowPass->use();
      traverseDrawLight(scene->root, Eigen::Affine3f::Identity());
      shadowPass->unuse();
      shadowBuffer->unbind();

      // PERFORM SHADING PASS
      accumulationBuffer->bind(0);
      shadingPass->use();
      shadingPass->uniform("power", power);

      const GLWrap::Texture2D *lightDepth = &shadowBuffer->depthTexture();
      lightDepth->bindToTextureUnit(4);
      shadingPass->uniform("lightDepth", 4);
      shadingPass->uniform("mVLight", lightCam.getViewMatrix().matrix());
      shadingPass->uniform("mPLight", lightCam.getProjectionMatrix().matrix());

      glClearColor(0, 0, 0, 0);
      glEnable(GL_BLEND);
      glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
      glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
      fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
      glDisable(GL_BLEND);
      shadingPass->unuse();
      accumulationBuffer->unbind();
    }
  }

  for (int i = 0; i < node->children.size(); i++)
  {
    accumulateLightsDeferred(node->children[i], acc);
  }
}

void RastApp::traverseDrawDeferred(shared_ptr<Node> node, Eigen::Affine3f acc)
{
  acc = acc * node->transform;
  geometryPass->uniform("mM", acc.matrix());

  for (int i = 0; i < node->meshInfos.size(); i++)
  {
    shared_ptr<nori::BSDF> mat = node->meshInfos[i]->material;
    shared_ptr<nori::Microfacet> micro = dynamic_pointer_cast<nori::Microfacet>(mat);

    float alpha = micro->alpha();
    float eta = micro->eta();

    Eigen::Vector3f kd = micro->diffuseReflectance();
    geometryPass->uniform("alpha", alpha);
    geometryPass->uniform("eta", eta);
    geometryPass->uniform("k_d", kd);

    node->meshInfos[i]->mesh->drawElements();
  }

  for (int i = 0; i < node->children.size(); i++)
  {
    traverseDrawDeferred(node->children[i], acc);
  }
}

void RastApp::drawContents()
{

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  // FORWARD SHADING
  if (!useDeferred)
  {
    GLWrap::checkGLError("drawContents start");
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    forwardShader->use();
    forwardShader->uniform("mM", Eigen::Affine3f::Identity().matrix());
    forwardShader->uniform("mV", cam->getViewMatrix().matrix());
    forwardShader->uniform("mP", cam->getProjectionMatrix().matrix());
    forwardShader->uniform("lightPos", Eigen::Vector3f(2., 2., 2.));
    forwardShader->uniform("power", Eigen::Vector3f(500., 500., 500.));

    findLightsForward(scene->root, Eigen::Affine3f::Identity());
    traverseDrawForward(scene->root, Eigen::Affine3f::Identity());
    forwardShader->unuse();
  }

  // DEFERRED SHADING
  else
  {

    // GEOMETRY PASS
    geometryBuffer->bind(0);
    const GLenum buffers[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffers);

    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    geometryPass->use();
    geometryPass->uniform("mM", Eigen::Affine3f::Identity().matrix());
    geometryPass->uniform("mV", cam->getViewMatrix().matrix());
    geometryPass->uniform("mP", cam->getProjectionMatrix().matrix());

    traverseDrawDeferred(scene->root, Eigen::Affine3f::Identity());
    geometryPass->unuse();
    geometryBuffer->unbind();

    const GLWrap::Texture2D *tex0 = &geometryBuffer->colorTexture(0);
    glBindTexture(GL_TEXTURE_2D, tex0->id());
    const GLWrap::Texture2D *tex1 = &geometryBuffer->colorTexture(1);
    glBindTexture(GL_TEXTURE_2D, tex1->id());
    const GLWrap::Texture2D *tex2 = &geometryBuffer->colorTexture(2);
    glBindTexture(GL_TEXTURE_2D, tex2->id());

    const GLWrap::Texture2D *d = &geometryBuffer->depthTexture();
    glBindTexture(GL_TEXTURE_2D, d->id());

    tex0->bindToTextureUnit(0);
    tex1->bindToTextureUnit(1);
    tex2->bindToTextureUnit(2);
    d->bindToTextureUnit(3);

    shadingPass->uniform("image0", 0);
    shadingPass->uniform("image1", 1);
    shadingPass->uniform("image2", 2);
    shadingPass->uniform("depth", 3);
    shadingPass->uniform("mV", cam->getViewMatrix().matrix());
    shadingPass->uniform("mVi", cam->getViewMatrix().inverse().matrix());
    shadingPass->uniform("mPi", cam->getProjectionMatrix().inverse().matrix());

    accumulationBuffer->bind(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    accumulationBuffer->unbind();

    // LOOP THROUGH LIGHTS FOR SHADING/SHADOW PASS
    accumulateLightsDeferred(scene->root, Eigen::Affine3f::Identity());

    // START AMBIENT PASS
    // loop through root lights looking for ambient light
    for (int i = 0; i < scene->root->lightInfos.size(); i++)
    {
      shared_ptr<RTUtil::LightInfo> ambLight = scene->root->lightInfos[i];
      if (ambLight->type == RTUtil::Ambient)
      {

        tex0->bindToTextureUnit(1);
        tex2->bindToTextureUnit(2);
        d->bindToTextureUnit(3);

        ambientPass->uniform("diffuse", 1);
        ambientPass->uniform("normal", 2);
        ambientPass->uniform("depth", 3);
        ambientPass->uniform("radiance", ambLight->radiance);
        ambientPass->uniform("range", ambLight->range);
        ambientPass->uniform("mP", cam->getProjectionMatrix().matrix());
        ambientPass->uniform("mV", cam->getViewMatrix().matrix());
        // ambientPass->uniform("mVi", cam->getViewMatrix().matrix().inverse());
        sky.setUniforms(*ambientPass);
        accumulationBuffer->bind(0);
        ambientPass->use();
        glClearColor(0, 0, 0, 0);
        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
        fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisable(GL_BLEND);
        ambientPass->unuse();
        accumulationBuffer->unbind();
      }
    }
    // END AMBIENT PASS

    const GLWrap::Texture2D *img = &accumulationBuffer->colorTexture();
    glBindTexture(GL_TEXTURE_2D, img->id());
    img->setParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    const GLWrap::Texture2D *temp = &blurBuffer->colorTexture(1);
    glBindTexture(GL_TEXTURE_2D, temp->id());
    temp->setParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    const GLWrap::Texture2D *bl = &blurBuffer->colorTexture(0);
    glBindTexture(GL_TEXTURE_2D, bl->id());
    bl->setParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // BLUR PASS
    const GLenum tempBuffer[]{GL_COLOR_ATTACHMENT1};
    const GLenum outBuffer[]{GL_COLOR_ATTACHMENT0};

    for (int i = 0; i < 4; i++)
    {
      img->bindToTextureUnit(0);
      blurBuffer->bind(ks[i]);
      //blurBuffer->bind(0);

      Eigen::Vector2i buffSize;
      glfwGetFramebufferSize(glfwWindow(), &buffSize.x(), &buffSize.y());
      glViewport(0, 0, buffSize.x() / float(pow(2, ks[i])), buffSize.y() / float(pow(2, ks[i])));

      glDrawBuffers(1, tempBuffer);

      float stddev = sigmas[i] / float(pow(2, ks[i]));
      float rad = stddev * 4;
      blurPass->uniform("stdev", stddev);
      blurPass->uniform("radius", int(rad));
      blurPass->uniform("dir", Eigen::Vector2f(0, 1)); // vertical
      blurPass->uniform("level", ks[i]);
      blurPass->uniform("image", 0);

      blurPass->use();
      fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
      blurPass->unuse();

      const GLWrap::Texture2D *blurred = &blurBuffer->colorTexture(1);
      glBindTexture(GL_TEXTURE_2D, blurred->id());
      blurred->bindToTextureUnit(0);

      glDrawBuffers(1, outBuffer);

      blurPass->uniform("dir", Eigen::Vector2f(1, 0)); // horizontal

      blurPass->use();
      fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
      blurPass->unuse();
      blurBuffer->unbind();
    }

    // MERGE PASS
    bl->bindToTextureUnit(1);
    img->bindToTextureUnit(0);
    mergeBuffer->bind(0);
    Eigen::Vector2i buffSize;
    glfwGetFramebufferSize(glfwWindow(), &buffSize.x(), &buffSize.y());
    glViewport(0, 0, buffSize.x(), buffSize.y());

    mergePass->uniform("levels", Eigen::Vector4f(ks[0], ks[1], ks[2], ks[3]));
    mergePass->uniform("mipmap", 1);
    mergePass->uniform("original", 0);

    mergePass->use();
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    mergePass->unuse();
    mergeBuffer->unbind();

    const GLWrap::Texture2D *bloom = &mergeBuffer->colorTexture(0);
    // bloom->setParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    bloom->bindToTextureUnit(0);
    //bl->bindToTextureUnit(0);

    // POST PROCESSING PASS
    postProcessingPass->use();
    postProcessingPass->uniform("image", 0);
    postProcessingPass->uniform("convertToSRGB", 1);
    postProcessingPass->uniform("exposure", float(1.0));

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    postProcessingPass->unuse();
  }
}

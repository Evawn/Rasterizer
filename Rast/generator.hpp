#include <RTUtil/Camera.hpp>
#include <RTUtil/sceneinfo.hpp>
#include <RTUtil/conversions.hpp>
#include <Eigen/Core>
#include <GLWrap/Mesh.hpp>

#include <ext/assimp/include/assimp/Importer.hpp>  // Plain-C interface
#include <ext/assimp/include/assimp/scene.h>       // Output data structure
#include <ext/assimp/include/assimp/postprocess.h> // Post processing flags

using namespace std;

class MeshInfo
{
public:
  shared_ptr<GLWrap::Mesh> mesh;
  shared_ptr<nori::BSDF> material;
};

class Node
{
public:
  Node();

  Eigen::Affine3f transform;
  vector<shared_ptr<MeshInfo> > meshInfos;
  vector<shared_ptr<RTUtil::LightInfo> > lightInfos;
  vector<shared_ptr<Node> > children;
  shared_ptr<Node> parent;
};

class Scene
{
public:
  Scene();

  RTUtil::PerspectiveCamera cam;
  shared_ptr<Node> root;
  RTUtil::SceneInfo info;
  // int numMeshes = 0;
  shared_ptr<nori::BSDF> default_mat;
  Eigen::Vector3f bgColor;
  // map<int, shared_ptr<nori::BSDF> > materials;
  //TO ADD: lights
};

class Generator
{
  static void initializeScene(shared_ptr<Scene> scene, const aiScene *data);
  static void traverseNodes(shared_ptr<Scene> scene, const aiScene *data, shared_ptr<Node> parent, shared_ptr<Node> myNode, aiNode *node);

  static void initializeCamera(shared_ptr<Scene> scene, const aiScene *data);

public:
  static shared_ptr<Scene> generateScene(const string &filename);
};
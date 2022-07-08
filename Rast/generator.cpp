#include "generator.hpp"

using namespace std;

string resourcePath = "../resources/scenes/";
string infoFile = "_info.json";
string meshFile = ".dae";

// MeshInfo::MeshInfo()
// {
// }

Node::Node()
{
  this->transform = Eigen::Affine3f::Identity();
  this->children = vector<shared_ptr<Node> >();
}

Scene::Scene() : cam(
                     Eigen::Vector3f(6, 8, 10), // eye
                     Eigen::Vector3f(0, 0, 0),  // target
                     Eigen::Vector3f(0, 1, 0),  // up
                     4. / 3.,                   // aspect
                     0.1, 50.0,                 // near, far
                     15.0 * M_PI / 180          // fov
                 ){};

void Generator::initializeScene(shared_ptr<Scene> scene, const aiScene *data)
{
  cout << "Initializing scene...\n";
  aiNode *node = data->mRootNode;
  scene->root = make_shared<Node>();
  traverseNodes(scene, data, nullptr, scene->root, node);
}

//initializes all info of a node, intializes children
void Generator::traverseNodes(shared_ptr<Scene> scene, const aiScene *data, shared_ptr<Node> parent, shared_ptr<Node> myNode, aiNode *aNode)
{
  cout << "Traversing nodes...\n";
  Eigen::Affine3f T = RTUtil::a2e(aNode->mTransformation);
  myNode->transform = T;
  myNode->parent = parent;

  // for each mesh...
  for (int m = 0; m < aNode->mNumMeshes; m++)
  {
    cout << "Mesh " << m << "\n";
    // make a mesh

    cout << "Mesh " << m << "\n";

    //-----
    unsigned int meshNum = aNode->mMeshes[m];
    aiMesh *mesh = data->mMeshes[meshNum];

    int nVerts = mesh->mNumVertices;
    int nFaces = mesh->mNumFaces;
    //-----

    // add verts to mesh
    Eigen::Matrix<float, 3, Eigen::Dynamic> positions(3, nVerts);
    for (int i = 0; i < nVerts; i++)
    {
      Eigen::Vector3f vert = RTUtil::a2e(mesh->mVertices[i]);
      positions.col(i) << vert.x(), vert.y(), vert.z();
    }

    shared_ptr<GLWrap::Mesh> curr_mesh = make_shared<GLWrap::Mesh>();
    curr_mesh->setAttribute(0, positions);

    // add triangles to mesh
    Eigen::VectorXi indices(3 * nFaces);
    for (int i = 0; i < nFaces; i++)
    {
      indices(3 * i) = mesh->mFaces[i].mIndices[0];
      indices(3 * i + 1) = mesh->mFaces[i].mIndices[1];
      indices(3 * i + 2) = mesh->mFaces[i].mIndices[2];
    }
    curr_mesh->setIndices(indices, GL_TRIANGLES);

    // add normals to mesh
    Eigen::Matrix<float, 3, Eigen::Dynamic> normals(3, nVerts);
    for (int i = 0; i < nVerts; i++)
    {
      Eigen::Vector3f normal = RTUtil::a2e(mesh->mNormals[i]);
      normals.col(i) << normal.x(), normal.y(), normal.z();
    }
    curr_mesh->setAttribute(1, normals);

    // Add the material for the geometry
    // If the mesh has a material and there is a BSDF in the scene info with a "name" field matching the material name, use it.
    // Otherwise, if the mesh is below a node whose name matches the "node" field of a material in the scene info, use it.
    // Otherwise use the scene's default material.

    // add the mesh to the node
    shared_ptr<MeshInfo> meshi = make_shared<MeshInfo>();
    meshi->mesh = curr_mesh;

    aiString meshMat = data->mMaterials[mesh->mMaterialIndex]->GetName();
    try
    {
      std::shared_ptr<nori::BSDF> mat = scene->info.namedMaterials.at(meshMat.C_Str());
      meshi->material = mat;
    }
    catch (...)
    {
      try
      {
        std::shared_ptr<nori::BSDF> mat = scene->info.nodeMaterials.at(aNode->mName.C_Str());
        meshi->material = mat;
      }
      catch (...)
      {
        std::shared_ptr<nori::BSDF> mat = scene->info.defaultMaterial;
        meshi->material = mat;
      }
    }

    myNode->meshInfos.push_back(meshi);
  }

  // Parse through the scene info
  // Look for lights under node
  for (int i = 0; i < scene->info.lights.size(); i++)
  {
    std::shared_ptr<RTUtil::LightInfo> l = scene->info.lights[i];

    if (aNode->mName.C_Str() == l->nodeName)
    {
      if (l->type == RTUtil::Point)
      {
        myNode->lightInfos.push_back(l);
        std::cout << "Added a point light \n";
      }
    }
  }

  // Recurse through the children nodes
  for (int c = 0; c < aNode->mNumChildren; c++)
  {
    shared_ptr<Node> newNode = make_shared<Node>();
    // weak_ptr<Node> weakyboi = newNode;
    myNode->children.push_back(newNode);
    traverseNodes(
        scene,
        data,
        myNode,
        newNode,
        aNode->mChildren[c]);
  }
  cout << "Children: " << myNode->children.size() << "\n";
  cout << "Meshes: " << myNode->meshInfos.size() << "\n";
}

void Generator::initializeCamera(shared_ptr<Scene> scene, const aiScene *data)
{
  if (data->HasCameras())
  {

    aiCamera *camData = data->mCameras[0];
    aiString camNodeName = camData->mName;

    aiNode *curNode = data->mRootNode->FindNode(camNodeName);

    //Eigen::IOFormat format(Eigen::StreamPrecision, 0, ", ", "; ", "", "", "[", "]\n");

    Eigen::Affine3f M = Eigen::Affine3f::Identity();
    while (curNode != NULL)
    {
      M = RTUtil::a2e(curNode->mTransformation) * M;
      curNode = curNode->mParent;
    }

    Eigen::Vector3f camPosWorld = M * RTUtil::a2e(camData->mPosition);
    Eigen::Vector3f camLookAtWorld = M.linear() * RTUtil::a2e(camData->mLookAt);
    Eigen::Vector3f camUpWorld = M.linear() * RTUtil::a2e(camData->mUp);

    float t = (-1 * camPosWorld).dot(camLookAtWorld) / camLookAtWorld.dot(camLookAtWorld) / 3;
    Eigen::Vector3f tar = camPosWorld + camLookAtWorld * 10.; // * t;

    float vfov = 2 * tan(atan(camData->mHorizontalFOV / 2) / camData->mAspect);

    RTUtil::PerspectiveCamera camera = RTUtil::PerspectiveCamera(
        camPosWorld, // eye
        tar,
        camUpWorld,       // up
        camData->mAspect, // aspect
        camData->mClipPlaneNear,
        camData->mClipPlaneFar, // near, far
        vfov                    // fov
    );

    scene->cam = camera;
  }
}

shared_ptr<Scene> Generator::generateScene(const string &filename)
{
  string base = filename.substr(0, filename.find('.'));
  cout << base << '\n';

  // create pointer to scene
  shared_ptr<Scene> scene = make_shared<Scene>();
  // scene.reset(new Scene());

  // fill in info
  RTUtil::SceneInfo info;
  std::cout << RTUtil::readSceneInfo(resourcePath + base + infoFile, info);
  scene->info = info;

  // NEED TO ADD MATERIAL STUFF HERE
  scene->default_mat = info.defaultMaterial;

  // Import the scene files
  Assimp::Importer importer;
  const aiScene *data = importer.ReadFile(
      resourcePath + filename,
      aiProcess_CalcTangentSpace |
          aiProcess_Triangulate |
          aiProcess_JoinIdenticalVertices |
          aiProcess_SortByPType |
          aiProcess_GenNormals);

  if (!data)
  {
    printf("Scene failed in import!\n");
  }
  else
  {
    printf("Scene imported successfully\n");
  }

  //INITIALIZE CAMERA
  //INITIALIZE SCENE
  initializeScene(scene, data);
  initializeCamera(scene, data);

  // Add ambient lights
  for (int i = 0; i < scene->info.lights.size(); i++)
  {
    std::shared_ptr<RTUtil::LightInfo> l = scene->info.lights[i];

    if (l->type == RTUtil::Ambient)
    {
      scene->root->lightInfos.push_back(l);
      scene->bgColor = l->radiance;
      std::cout << "Added an ambient light \n";
    }
  }

  importer.FreeScene();
  return scene;
}
#include "OgreManager.h"

#include "MainWindow.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreDataStream.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreMeshSerializer.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubMesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

static OgreManager *_instance = 0;

class OgreManagerPrivate {
public:
  OgreManagerPrivate() : root(0), sceneManager(0) {
  }

  ~OgreManagerPrivate() {
    delete root;
  }

  void updateViews() {
    for (int i = 0; i < windows.size(); ++i)
      windows.at(i)->update(true);
  }

  Ogre::Root *root;
  Ogre::SceneManager *sceneManager;
  QList<Ogre::RenderWindow *> windows;
  Ogre::SceneNode *floorNode;
  Ogre::SceneNode *ceilingNode;
  Ogre::Entity *floor;
  Ogre::Entity *ceiling;
};

OgreManager::OgreManager(MainWindow *parent) : QObject(parent), d(new OgreManagerPrivate()) {
  _instance = this;
  // initialize OGRE
  d->root = new Ogre::Root("", "", "");
  // load plugins
#ifdef Q_WS_X11
  d->root->loadPlugin("/usr/lib/OGRE/RenderSystem_GL.so");
#endif
#ifdef USE_OPENGL
  d->root->loadPlugin("RenderSystem_GL.dll");
#endif
#ifdef USE_DIRECTX
  d->root->loadPlugin("RenderSystem_Direct3D9.dll");
#endif
  // select the rendersystem
  d->root->setRenderSystem(d->root->getAvailableRenderers().at(0));
  // initialise root object
  d->root->initialise(false);
  // create 1x1 top level window
  createWindow(parent, 1, 1);
  // add resource locations
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/scripts", "FileSystem", "General");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/elements/meshes", "FileSystem", "General");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/textures", "FileSystem", "General");
  // load resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  // create the scene manager
  d->sceneManager = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC);
  // use stencil shadows
  d->sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
  // set up ambient light
  d->sceneManager->setAmbientLight(Ogre::ColourValue(0.1f, 0.1f, 0.1f));
  // TODO: create floor and ceiling objects such that the center of the texture is the center of the plane
  // create floor object
  Ogre::MeshManager::getSingleton().createPlane("Floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 10000, 10000, 50, 50, true, 1, 100.0f, 100.0f, Ogre::Vector3::UNIT_Z);
  // create the floor object
  d->floor = d->sceneManager->createEntity("Floor");
  d->floor->setMaterialName("Floor/1");
  d->floor->setCastShadows(false);
  // create floor node
  d->floorNode = d->sceneManager->getRootSceneNode()->createChildSceneNode();
  d->floorNode->attachObject(d->floor);
  // create ceiling object
  Ogre::MeshManager::getSingleton().createPlane("Ceiling", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::Plane(-Ogre::Vector3::UNIT_Y, 0), 10000, 10000, 50, 50, true, 1, 100.0f, 100.0f, Ogre::Vector3::UNIT_Z);
  d->ceiling = d->sceneManager->createEntity("Ceiling");
  d->ceiling->setMaterialName("Ceiling/White");
  d->ceiling->setCastShadows(false);
  // create the ceiling node
  d->ceilingNode = d->sceneManager->getRootSceneNode()->createChildSceneNode();
  d->ceilingNode->setPosition(0.0f, 300.0f, 0.0f);
  d->ceilingNode->attachObject(d->ceiling);
  // create a light
  Ogre::Light *light = OgreManager::instance()->sceneManager()->createLight();
  light->setType(Ogre::Light::LT_DIRECTIONAL);
  light->setDiffuseColour(0.5f, 0.5f, 0.5f);
  light->setSpecularColour(1.0f, 1.0f, 1.0f);
  // attach the light to the scene
  OgreManager::instance()->sceneManager()->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0.0f, 295.0f, 0.0f))->attachObject(light);
}

OgreManager::~OgreManager() {
  delete d;
}

OgreManager *OgreManager::instance() {
  return _instance;
}

Ogre::SceneManager *OgreManager::sceneManager() {
  return d->sceneManager;
}

Ogre::RenderWindow *OgreManager::createWindow(QWidget *widget, int width, int height) {
  // create render window
  Ogre::NameValuePairList options;
#ifdef Q_WS_X11
  const QX11Info &info = widget->x11Info();
  options["externalWindowHandle"] = QString("%1:%2:%3")
                                    .arg((unsigned long)info.display())
                                    .arg((unsigned int)info.screen())
                                    .arg((unsigned long)widget->winId()).toStdString();
#else
  options["externalWindowHandle"] = QString("%1").arg((unsigned long)widget->winId()).toStdString();
#endif
  options["FSAA"] = "4";
  // create a new window
  Ogre::RenderWindow *window = d->root->createRenderWindow(QString("OgreWindow-%1").arg(d->windows.size()).toStdString(), width, height, false, &options);
  // save window
  d->windows.append(window);
  // return created window
  return window;
}

Ogre::Camera *OgreManager::createCamera(const QString &name) {
  return d->sceneManager->createCamera(name.toStdString());
}

const QString OgreManager::supportedFormats() const {
  std::string extensions;
  // get supported extensions
  Assimp::Importer().GetExtensionList(extensions);
  // return as qstring
  return QString::fromStdString(extensions).replace(";", " ").prepend("*.mesh ");
}

Ogre::Entity *OgreManager::loadMesh(const QString &path) {
  Ogre::String source = path.toStdString();
  // check file extension
  if (path.toLower().endsWith(".mesh")) {
    // load the mesh file content
    Ogre::DataStreamPtr stream(new Ogre::FileStreamDataStream(new std::ifstream(source.c_str())));
    // create the mesh pointer
    Ogre::MeshPtr meshPtr = Ogre::MeshManager::getSingletonPtr()->createManual(source, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    // import mesh content into the mesh pointer
    Ogre::MeshSerializer().importMesh(stream, meshPtr.getPointer());
  } else {
    // try to loading the mesh using assimp
    Assimp::Importer *importer = new Assimp::Importer();
    const aiScene *scene = importer->ReadFile(source.c_str(),
                           aiProcess_CalcTangentSpace |
                           aiProcess_JoinIdenticalVertices |
                           aiProcess_Triangulate |
                           // aiProcess_RemoveComponent |
                           // aiProcess_GenNormals |
                           aiProcess_GenSmoothNormals |
                           // aiProcess_SplitLargeMeshes |
                           // aiProcess_PreTransformVertices |
                           // aiProcess_LimitBoneWeights |
                           // aiProcess_ValidateDataStructure |
                           aiProcess_ImproveCacheLocality |
                           aiProcess_RemoveRedundantMaterials |
                           aiProcess_FixInfacingNormals |
                           aiProcess_SortByPType |
                           aiProcess_FindDegenerates |
                           aiProcess_FindInvalidData |
                           aiProcess_GenUVCoords |
                           aiProcess_TransformUVCoords |
                           // aiProcess_FindInstances |
                           aiProcess_OptimizeMeshes |
                           aiProcess_OptimizeGraph |
                           aiProcess_FlipUVs
                           // aiProcess_FlipWindingOrder |
                           // aiProcess_SplitByBoneCount |
                           // aiProcess_Debone
                                             );
    if (scene->HasMaterials()) {
      // TODO: load materials
    }
    Ogre::MeshPtr meshPtr = Ogre::MeshManager::getSingletonPtr()->createManual(source, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::AxisAlignedBox aabb;
    if (scene->HasMeshes()) {
      for (int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh *aimesh = scene->mMeshes[i];
        // primitive contains primitives other than triangles skip
        // should be use together with aiProcess_SortByPType
        if (aimesh->mPrimitiveTypes & (aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON))
          continue;
        // create a submesh
        Ogre::SubMesh *submesh = meshPtr->createSubMesh();
        // create vertex buffer
        submesh->useSharedVertices = false;
        submesh->vertexData = new Ogre::VertexData();
        submesh->vertexData->vertexStart = 0;
        submesh->vertexData->vertexCount = aimesh->mNumVertices;
        // create vertex declaration
        Ogre::VertexDeclaration* declaration = submesh->vertexData->vertexDeclaration;
        unsigned short start = 0;
        size_t offset = 0;
        // get pointers to the position, normal and texture coordinates arrays
        aiVector3D *position = aimesh->mVertices;
        aiVector3D *normal = aimesh->mNormals;
        aiVector3D *texCoord = aimesh->mTextureCoords[0];
        // create position element
        if (position)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
        // create normal element
        if (normal)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL).getSize();
        // create texture coords element
        if (texCoord)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES).getSize();
        // create vertex buffer
        Ogre::HardwareVertexBufferSharedPtr vertexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(declaration->getVertexSize(start),
            submesh->vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        // get vertex buffer
        float* vertexData = static_cast<float*>(vertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        // parse vertices
        for (size_t j = 0; j < aimesh->mNumVertices; ++j) {
          // vertex position
          if (aimesh->HasPositions()) {
            *vertexData++ = position->x;
            *vertexData++ = position->y;
            *vertexData++ = position->z;
            // next vertex
            position++;
            // update bounding box
            aabb.merge(Ogre::Vector3(position->x, position->y, position->z));
          }
          // vertex normal
          if (aimesh->HasNormals()) {
            *vertexData++ = normal->x;
            *vertexData++ = normal->y;
            *vertexData++ = normal->z;
            // next vertex
            normal++;
          }
          // texture coordinates
          if (aimesh->HasTextureCoords(0)) {
            *vertexData++ = texCoord->x;
            *vertexData++ = texCoord->y;
            // next vertex
            texCoord++;
          }
        }
        vertexBuffer->unlock();
        // bind the buffer
        submesh->vertexData->vertexBufferBinding->setBinding(start, vertexBuffer);
        // create index data
        submesh->indexData->indexStart = 0;
        submesh->indexData->indexCount = aimesh->mNumFaces * 3;
        // get pointer to the face data
        aiFace *f = aimesh->mFaces;
        // index buffer
        submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createIndexBuffer(Ogre::HardwareIndexBuffer::IT_32BIT,
                                          submesh->indexData->indexCount,
                                          Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        Ogre::uint32* idata = static_cast<Ogre::uint32*>(submesh->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        // faces
        for (size_t j = 0; j < aimesh->mNumFaces; ++j) {
          *idata++ = f->mIndices[0];
          *idata++ = f->mIndices[1];
          *idata++ = f->mIndices[2];
          // next face
          f++;
        }
        submesh->indexData->indexBuffer->unlock();
        // TODO: assign material
      }
    }
    meshPtr->_setBounds(aabb);
    meshPtr->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() * 0.5f);
    // clean up
    delete importer;
  }
  // create and return an entity from the mesh
  return d->sceneManager->createEntity(source);
}

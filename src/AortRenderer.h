#ifndef AORTRENDERER_H
#define AORTRENDERER_H

#include <QImage>

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class RendererPrivate;

  class Renderer {
  public:
    Renderer();
    ~Renderer();

    QImage render(Ogre::SceneNode *root, const Ogre::Camera *camera, const int width, const int height);

  private:
    RendererPrivate *d;
  };
}

#endif // AORTRENDERER_H
#ifndef AORTLIGHT_H
#define AORTLIGHT_H

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  enum LightType {
    LT_POINT,
    LT_AREA
  };

  class LightPrivate;

  class Light {
  public:
    Light();
    ~Light();

    const LightType &getType() const;
    void setType(const LightType &type);

    const Ogre::ColourValue &getDiffuseColour() const;
    void setDiffuseColour(const Ogre::ColourValue &colour);

    const Ogre::ColourValue &getSpecularColour() const;
    void setSpecularColour(const Ogre::ColourValue &colour);

    const Ogre::Vector3 &getPosition() const;
    void setPosition(const Ogre::Vector3 &position);

    const Ogre::Vector3 &getDirection() const;
    void setDirection(const Ogre::Vector3 &direction);

    const Ogre::Vector2 &getSize() const;
    void setSize(const Ogre::Vector2 &size);

    const Ogre::Vector3 *getPoints() const;

  private:
    LightPrivate *d;
  };
}

#endif // AORTLIGHT_H

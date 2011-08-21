#ifndef KDTREE_H
#define KDTREE_H

#include <OGRE/OgrePrerequisites.h>

#include <float.h>

#define MAXIMUM_DEPTH (64)
#define MINIMUM_TRIANGLES_PER_LEAF (2)

class Triangle;

namespace Ogre {
  class AxisAlignedBox;
  class Ray;
}

class KdTreeNode {
public:
  KdTreeNode();
  KdTreeNode(const Ogre::AxisAlignedBox &aabb, const void *pointer);

  const bool hit(const Ogre::Ray &ray, Triangle *&triangle, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v, const Ogre::Real t_min = 0.0f, const Ogre::Real t_max = FLT_MAX) const;
  const bool hit(const Ogre::Ray &ray, const Ogre::Real t_min = 0.0f, const Ogre::Real t_max = FLT_MAX) const;

private:
  void split(const Ogre::AxisAlignedBox &aabb, const int depth);

  const bool isLeaf() const;
  void setLeaf(const bool leaf);

  const int axis() const;
  void setAxis(const int axis);

  KdTreeNode *nodes() const;
  Triangle **triangles() const;

  void setPointer(const void *pointer);

private:
  unsigned long data;
  Ogre::Real splitPosition;
};

#endif // KDTREE_H

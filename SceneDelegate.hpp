//================================================================================================
// Name         : SceneDelegate.hpp
// Author(s)    : Tyrone Davison
// Version      : 1.0
// Copyright    : Tyrone Davison, Teesside University, 2011
// Description  : Model class to provide mesh, shape, and light scene data.
// Instructions : Do not modify this file.
//================================================================================================

#pragma once
#ifndef __SCENE_DELEGATE__
#define __SCENE_DELEGATE__

#include <vector>

class Float2 {
public:
  float x;
  float y;
public:
  Float2() : x(0), y(0) {}
  Float2(float X, float Y) : x(X), y(Y) {}
};

class Float3 {
public:
  float x;
  float y;
  float z;
public:
  Float3() : x(0), y(0), z(0) {}
  Float3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
  Float3(const Float2& f2, float Z) : x(f2.x), y(f2.y), z(Z) {}
};

class Float4 {
public:
  float x;
  float y;
  float z;
  float w;
public:
  Float4() : x(0), y(0), z(0), w(1) {}
  Float4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
  Float4(const Float2& f2, float Z, float W) : x(f2.x), y(f2.y), z(Z), w(W) {}
  Float4(const Float3& f3, float W) : x(f3.x), y(f3.y), z(f3.z), w(W) {}
};

struct Mesh {
  std::vector<Float3> vertexArray;
  std::vector<Float3> normalArray;
  std::vector<unsigned int> indexArray;
};

struct Shape {
  unsigned int meshIndex;
  Float3 position;
  Float3 rotation; // radians
};

struct Light {
  float coneAngle; // radians
  Float3 position;
  Float3 direction;
  float intensity;
};

class SceneDelegate {
public:
  SceneDelegate(void);
  ~SceneDelegate(void);
public:
  void animate(float time);
public:
  Float3 worldUpDirection(void) const;
  unsigned int numberOfMeshes(void) const;
  void getMeshAtIndex(unsigned int, Mesh*) const;
  unsigned int numberOfShapes(void) const;
  Shape shapeAtIndex(unsigned int) const;
  unsigned int numberOfLights(void) const;
  Light lightAtIndex(unsigned int) const;
private:
  std::vector<Light> light_;
  std::vector<Mesh> mesh_;
  std::vector<Shape> shape_;
};

#endif

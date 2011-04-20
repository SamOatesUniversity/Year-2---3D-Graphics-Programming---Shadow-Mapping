/*
  Copyright (c) 2011 Tyrone Davison

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#pragma once
#ifndef __TSL_PRIMITIVES__
#define __TSL_PRIMITIVES__

#include "geometry.hpp"

namespace tsl
{
  void CreateBox( float X, float Y, float Z, unsigned int subdivisions, IndexedMesh* out );
  void CreateCube( float size, unsigned int subdivisions, IndexedMesh* out );
  void CreateSphere( float radius, unsigned int subdivisions, IndexedMesh* out );
  void CreateCylinder( float radius, float height, unsigned int subdivisions, IndexedMesh* out );
  void CreateCone( float radius, float height, unsigned int subdivisions, IndexedMesh* out );
  void CreateDisk( float radius, float height, unsigned int subdivisions, IndexedMesh* out );
}

#endif

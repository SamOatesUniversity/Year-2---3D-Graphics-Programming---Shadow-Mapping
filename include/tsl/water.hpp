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
#ifndef __TSL_WATER__
#define __TSL_WATER__

#include <vector>

namespace tsl
{
  class Water
  {
  public:
    Water( float time_step, float size, int num_points, float scale_factor, float wind_direction, float wind_speed );
    ~Water();
  public:
    float Advance( const int num_steps, float* const position_array, const int position_stride, float* const normal_array, const int normal_stride );
    int IndexCount() const;
    void GetIndices( int*  index_array );
    int VertexCount() const;
  private:
    struct POINT;
  private:
    const float         dt;
    const int           N;
    const int           Nx;
    const int           Ny;
    const float         Lx;
    const float         Ly;
    std::vector<POINT>  mesh;
    std::vector<float>  H;
    std::vector<float>  nox, noy;
  };
}

#endif

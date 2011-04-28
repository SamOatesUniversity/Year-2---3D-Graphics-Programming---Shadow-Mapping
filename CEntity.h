#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>
#include "SceneDelegate.hpp"

#include "CFirstPersonCamera.h"
#include "CShader.h"
#include "CLight.h"

class CEntity {
private:

	IDirect3DVertexDeclaration9* _vertex_declaration; // specifies how the vertex buffer(s) are layed out

	// C++ version of our vertex layout
	struct Vertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
	};

	unsigned int _num_triangles; // the number of triangles in the shape
	unsigned int _num_vertices; // the number of vertices in the shape
	IDirect3DVertexBuffer9* _vertex_buffer; // holds vertex data for the shape
	IDirect3DIndexBuffer9* _index_buffer; // holds the index data for the shape

	CShader *_light, *_shadow, *_ambient;

	D3DXVECTOR3 _position, _rotation; // holds the entitys translations

	template<typename T>
	void Release(T** ptr) {
		if ( *ptr != 0 ) {
			(*ptr)->Release();
			*ptr = 0;
		}
	}

public:
	CEntity();
	~CEntity();
	bool init( IDirect3DDevice9 *dev, const Mesh *mesh, const Shape &shape );

	void drawAmbient( IDirect3DDevice9 *dev, CFirstPersonCamera *camera );
	void draw( IDirect3DDevice9 *dev, CFirstPersonCamera *camera, CLight *light, IDirect3DTexture9 *shadow_map );
	void drawShadows( IDirect3DDevice9 *dev, CLight *light );
	
	void update( const Shape &shape );

	void setshaders( CShader *light, CShader *shadow, CShader *ambient );

};
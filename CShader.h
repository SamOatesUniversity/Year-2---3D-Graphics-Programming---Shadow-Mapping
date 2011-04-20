#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>

class CShader {
private:
	IDirect3DVertexShader9* _vertex_shader; // the vertex shader compiled from file
	ID3DXConstantTable* _vertex_shader_constants; // a helper object configure the shader constants
	IDirect3DPixelShader9* _pixel_shader; // the pixel shader compiled from file
	ID3DXConstantTable* _pixel_shader_constants; // a helper object configure the shader constants

	template<typename T>
	void Release(T** ptr) {
		if ( *ptr != 0 ) {
			(*ptr)->Release();
			*ptr = 0;
		}
	}

public:
	CShader();
	~CShader();

	bool init( IDirect3DDevice9 *dev, char *vertex, char *pixel );

	IDirect3DVertexShader9	*vertex();
	ID3DXConstantTable		*vertex_constants();
	IDirect3DPixelShader9	*pixel();
	ID3DXConstantTable		*pixel_constants();
	
	bool isCompiled( void ) { return true; }

	void reload( IDirect3DDevice9 *dev, char *vertex, char *pixel );
};
#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>
#include "SceneDelegate.hpp"

//Just cos i <3 Microsoft
#define MAP_SIZE	2048

class CLight {
private:
	Light m_light;
public:
	CLight( Light light );
	~CLight( );

	D3DXVECTOR4 getPosition();
	D3DXMATRIX getViewProjection();

	Light getLight( void ) { return m_light; }

};

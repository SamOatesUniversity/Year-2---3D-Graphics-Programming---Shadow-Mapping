#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>
#include "SceneDelegate.hpp"

#define MAP_SIZE	1024

class CLight {
private:
	Light m_light;
	D3DXVECTOR3 m_up;
public:
	CLight( SceneDelegate* scene, int light_index );
	~CLight( );

	D3DXVECTOR4 getPosition();
	D3DXMATRIX getViewProjection();

	Light getLight( void ) { return m_light; }

};

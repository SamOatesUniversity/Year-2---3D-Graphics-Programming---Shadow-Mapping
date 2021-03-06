#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>

#include "SceneDelegate.hpp"

class CFirstPersonCamera
{
private:
	D3DXVECTOR3 m_position, m_rotation;
	D3DXVECTOR3 m_up, m_look, m_right, m_world_up;
	D3DXVECTOR2 m_last_mouse_position;
public:
	CFirstPersonCamera(void);
	~CFirstPersonCamera(void);

	void init( SceneDelegate* scene, D3DXVECTOR3 position, D3DXVECTOR3 rotation );

	D3DXVECTOR4 getPosition() { return D3DXVECTOR4( m_position, 1.0f ); }
	D3DXMATRIX ViewTransformation();
	D3DXMATRIX ProjectionTransformation( float viewport_aspect ) const;

	void move( D3DXVECTOR3 amount );
	void turn( D3DXVECTOR3 amount );
	void setRotation( D3DXVECTOR3 rot ) { m_rotation = rot; }

	void setLastMouseXY( int x, int y ) { m_last_mouse_position.x = (float)x; m_last_mouse_position.y = (float)y; }
	D3DXVECTOR2 getLastMouseXY( void ) { return m_last_mouse_position; }

};
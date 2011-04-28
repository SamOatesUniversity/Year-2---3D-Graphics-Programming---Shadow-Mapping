#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>

class CLookAtCamera {
private:
	void updatePosition( void );
	D3DXVECTOR3 m_position, m_lookat;
	float m_angleX, m_angleZ, m_distance;
	int m_lastMouseX, m_lastMouseY;

public:
	CLookAtCamera();
	~CLookAtCamera();
	bool init( float angle_x, float angle_z, float distance );

	D3DXMATRIX ViewTransformation( void ) const;
	D3DXMATRIX ProjectionTransformation( float viewport_aspect ) const;

	void rotateX( float angle );
	void rotateZ( float angle );
	void move( float amount );

	int getLastMouseX( void ) { return m_lastMouseX; }
	int getLastMouseY( void ) { return m_lastMouseY; }
	void setLastMouseXY( int x, int y ) { m_lastMouseX = x; m_lastMouseY = y; }

	D3DXVECTOR4 getPosition( void );
};
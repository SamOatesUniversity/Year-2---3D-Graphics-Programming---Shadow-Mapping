#include "CCamera.h"

CCamera::CCamera()
{
	m_position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_lookat = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_angleX = 1.0f;
	m_angleZ = 0.0f;
	m_distance = 10.0f;
	m_lastMouseX = 0, m_lastMouseY = 0;
}

CCamera::~CCamera()
{

}

bool CCamera::init( float angle_x, float angle_z, float distance )
{
	m_angleX = angle_x;
	m_angleZ = angle_z;
	m_distance = distance;
	updatePosition();
	return true;
}

D3DXVECTOR4 CCamera::getPosition( void )
{
	return D3DXVECTOR4( m_position, 1.0f );
}

void CCamera::updatePosition( void )
{
	m_position.x = (float)(m_distance * (cos( m_angleX ) * sin( m_angleZ ))) + m_lookat.x;
	m_position.y = -(float)(m_distance * (cos( m_angleX ) * cos( m_angleZ ))) + m_lookat.y;
	m_position.z = (float)(m_distance * sin( m_angleX )) + m_lookat.z;
}

void CCamera::rotateX( float angle )
{
	m_angleX += angle;
	updatePosition();
}

void CCamera::rotateZ( float angle )
{
	m_angleZ += angle;
	updatePosition();
}

void CCamera::move( float amount )
{
	m_distance -= amount;
	m_distance = m_distance < 0.0f ? 0.0f : m_distance;
	updatePosition();
}

D3DXMATRIX CCamera::ViewTransformation() const
{
	D3DXMATRIX view_xform;
	D3DXMatrixLookAtRH(	&view_xform,
		&m_position,    // the camera position
		&m_lookat,      // the look-at position
		&D3DXVECTOR3 (0.0f, 0.0f, 1.0f));    // the up direction
	return view_xform;
}

D3DXMATRIX CCamera::ProjectionTransformation( float viewport_aspect ) const
{
	D3DXMATRIX proj_xform;
	D3DXMatrixPerspectiveFovRH( &proj_xform, D3DX_PI * 0.33f, viewport_aspect, 0.1f, 100.0f );
	return proj_xform;
}
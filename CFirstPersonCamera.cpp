#include "CFirstPersonCamera.h"


CFirstPersonCamera::CFirstPersonCamera(void)
{
}

CFirstPersonCamera::~CFirstPersonCamera(void)
{
}

void CFirstPersonCamera::init( SceneDelegate* scene, D3DXVECTOR3 position, D3DXVECTOR3 rotation )
{
	m_position = position;
	m_rotation = rotation;
	m_world_up = D3DXVECTOR3(scene->worldUpDirection().x, scene->worldUpDirection().y, scene->worldUpDirection().z);
}

//Compute the view matrix
D3DXMATRIX CFirstPersonCamera::ViewTransformation()
{
	if( m_world_up.z > 0.0f )
	{
		m_up = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		m_look = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	}
	else
	{
		m_up = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		m_look = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	}
	m_right = D3DXVECTOR3( -1.0f, 0.0f, 0.0f );

	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis( &yawMatrix, &m_up, m_rotation.y );
	D3DXVec3TransformCoord( &m_look, &m_look, &yawMatrix );
	D3DXVec3TransformCoord( &m_right, &m_right, &yawMatrix ); 

	D3DXMATRIX pitchMatrix;
	D3DXMatrixRotationAxis( &pitchMatrix, &m_right, m_rotation.x );
	D3DXVec3TransformCoord( &m_look, &m_look, &pitchMatrix );
	D3DXVec3TransformCoord( &m_up, &m_up, &pitchMatrix ); 

	D3DXMATRIX rollMatrix;
	D3DXMatrixRotationAxis( &rollMatrix, &m_right, m_rotation.z );
	D3DXVec3TransformCoord( &m_right, &m_right, &rollMatrix );
	D3DXVec3TransformCoord( &m_up, &m_up, &rollMatrix ); 

	D3DXMATRIX viewMatrix;
	D3DXMatrixIdentity( &viewMatrix );

	//Frenet
	viewMatrix._11 = m_right.x; viewMatrix._12 = m_up.x; viewMatrix._13 = m_look.x;
	viewMatrix._21 = m_right.y; viewMatrix._22 = m_up.y; viewMatrix._23 = m_look.y;
	viewMatrix._31 = m_right.z; viewMatrix._32 = m_up.z; viewMatrix._33 = m_look.z;

	viewMatrix._41 = - D3DXVec3Dot( &m_position, &m_right );
	viewMatrix._42 = - D3DXVec3Dot( &m_position, &m_up );
	viewMatrix._43 = - D3DXVec3Dot( &m_position, &m_look );

	return viewMatrix;
}

//compute the projection matrix
D3DXMATRIX CFirstPersonCamera::ProjectionTransformation( float viewport_aspect ) const
{
	D3DXMATRIX proj_xform;
	D3DXMatrixPerspectiveFovRH( &proj_xform, D3DX_PI * 0.33f, viewport_aspect, 0.1f, 100.0f );
	return proj_xform;
}

void CFirstPersonCamera::move( D3DXVECTOR3 amount )
{
	m_position += m_look * -amount.z;
	m_position += m_right * amount.x;
	m_position += m_up * amount.y;
}

void CFirstPersonCamera::turn( D3DXVECTOR3 amount )
{
	m_rotation += amount;
}

#include "CLight.h"

CLight::CLight( SceneDelegate* scene, int light_index )
{
	m_light = scene->lightAtIndex( light_index );
	m_up = D3DXVECTOR3 (scene->worldUpDirection().x, scene->worldUpDirection().y, scene->worldUpDirection().z);
}

CLight::~CLight()
{
}

D3DXVECTOR4 CLight::getPosition( void )
{
	//returns the lights position as a vector4 as its easier
	//to pass into a shader
	return D3DXVECTOR4( m_light.position.x, m_light.position.y, m_light.position.z, 1.0f );
}

D3DXMATRIX CLight::getViewProjection()
{
	//Compute the lights view projection matrix 

	D3DXMATRIX view_xform;
	D3DXMatrixLookAtRH(	&view_xform,
		&D3DXVECTOR3( m_light.position.x, m_light.position.y, m_light.position.z ),    // the camera position
		&(D3DXVECTOR3( m_light.position.x, m_light.position.y, m_light.position.z ) + D3DXVECTOR3( m_light.direction.x, m_light.direction.y, m_light.direction.z )),      // the look-at position
		&m_up );    // the up direction

	D3DXMATRIX proj_xform;
	D3DXMatrixPerspectiveFovRH( &proj_xform, m_light.coneAngle * 2.0f, 1.0f, 0.1f, 100.0f );

	return view_xform * proj_xform;
}

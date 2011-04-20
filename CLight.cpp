#include "CLight.h"

CLight::CLight( Light light )
{
	m_light = light;
}

CLight::~CLight()
{
}

D3DXVECTOR4 CLight::getPosition( void )
{
	return D3DXVECTOR4( m_light.position.x, m_light.position.y, m_light.position.z, 1.0f );
}

D3DXMATRIX CLight::getViewProjection( void )
{
	D3DXMATRIX view_xform;
	D3DXMatrixLookAtRH(	&view_xform,
		&D3DXVECTOR3( m_light.position.x, m_light.position.y, m_light.position.z ),    // the camera position
		&(D3DXVECTOR3( m_light.position.x, m_light.position.y, m_light.position.z ) + D3DXVECTOR3( m_light.direction.x, m_light.direction.y, m_light.direction.z )),      // the look-at position
		&D3DXVECTOR3 (0.0f, 0.0f, 1.0f));    // the up direction

	D3DXMATRIX proj_xform;
	D3DXMatrixPerspectiveFovRH( &proj_xform, m_light.coneAngle * 2.0f, 1.0f, 0.1f, 100.0f );

	return view_xform * proj_xform;
}

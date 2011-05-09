#include "CEntity.h"

CEntity::CEntity()
{
	_vertex_declaration = NULL;
	_num_triangles = 0;
	_num_vertices = 0;
	_vertex_buffer = NULL;
	_index_buffer = NULL;
}

CEntity::~CEntity()
{
	Release( &_vertex_declaration );
	Release( &_vertex_buffer );
	Release( &_index_buffer );	
}

bool CEntity::init( IDirect3DDevice9 *dev, const Mesh *mesh, const Shape &shape )
{
	//Create a vertex decloration
	D3DVERTEXELEMENT9 vertex_elements[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, // an xyz position
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  // an xyz normal
		D3DDECL_END() 
	};
	dev->CreateVertexDeclaration( vertex_elements, &_vertex_declaration );

	//get the number of verticies
	_num_vertices = mesh->vertexArray.size();

	//reate a vertex buffer
	dev->CreateVertexBuffer( _num_vertices * sizeof(Vertex), 0, 0, D3DPOOL_MANAGED, &_vertex_buffer, 0 ); // create the vertex buffer

	// fill the vertex buffer with data
	Vertex* vptr;
	if ( _vertex_buffer && SUCCEEDED(_vertex_buffer->Lock( 0, 0, (void**)&vptr, 0 )) )
	{
		for ( UINT i = 0; i < _num_vertices; ++i )
		{
			vptr[i].position = D3DXVECTOR3( mesh->vertexArray[i].x, mesh->vertexArray[i].y, mesh->vertexArray[i].z );
			vptr[i].normal = D3DXVECTOR3( mesh->normalArray[i].x, mesh->normalArray[i].y, mesh->normalArray[i].z );
		}
		_vertex_buffer->Unlock();
	}

	//get the number of indexes
	_num_triangles = mesh->indexArray.size();

	// create the index buffer
	dev->CreateIndexBuffer( _num_triangles * 3 * sizeof(unsigned int), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &_index_buffer, 0 );

	// fill the index buffer
	UINT* iptr;
	if ( _index_buffer && SUCCEEDED(_index_buffer->Lock( 0, 0, (void**)&iptr, 0 )) )
	{
		for ( UINT i = 0; i < _num_triangles; ++i )
		{
			iptr[i] = mesh->indexArray[i];
		}
		_index_buffer->Unlock();
	}

	//position and rotate to initial transforms
	_position = D3DXVECTOR3( shape.position.x, shape.position.y, shape.position.z );
	_rotation = D3DXVECTOR3( shape.rotation.x, shape.rotation.y, shape.rotation.z );

	return true;	
}

void CEntity::drawAmbient( IDirect3DDevice9 *dev, CFirstPersonCamera *camera )
{
	D3DVIEWPORT9 viewport;
	dev->GetViewport( &viewport );

	D3DXMATRIX world_xform, rotation_x_xform, rotation_y_xform, rotation_z_xform, translation_xform;

	D3DXMatrixRotationX( &rotation_x_xform, _rotation.x );        // Pitch
	D3DXMatrixRotationY( &rotation_y_xform, _rotation.y );        // Yaw
	D3DXMatrixRotationZ( &rotation_z_xform, _rotation.z );        // Roll

	D3DXMatrixTranslation( &translation_xform, _position.x, _position.y, _position.z );

	world_xform = ( rotation_z_xform * rotation_x_xform * rotation_y_xform ) * translation_xform;

	// compute the view matrix using the camera object
	D3DXMATRIX view_xform = camera->ViewTransformation( );

	// compute the projection matrix using the camera object
	D3DXMATRIX projection_xform = camera->ProjectionTransformation( (float)viewport.Width / (float)viewport.Height );

	// combine the three pipeline matrices into a single matrix
	D3DXMATRIX world_view_projection_xform = world_xform * view_xform * projection_xform;

	// if shaders compiled correctly
	if ( _ambient->isCompiled() )
	{
		// configure the pipeline - primitive assembly
		dev->SetVertexDeclaration( _vertex_declaration );
		dev->SetStreamSource( 0, _vertex_buffer, 0, sizeof(Vertex) );
		dev->SetIndices( _index_buffer );

		// configure the pipeline - vertex shader

		IDirect3DVertexShader9 *_vertex_shader = _ambient->vertex();
		ID3DXConstantTable *_vertex_shader_constants = _ambient->vertex_constants();

		dev->SetVertexShader( _vertex_shader );
		_vertex_shader_constants->SetMatrix( dev, "world_xform", &world_xform );
		_vertex_shader_constants->SetMatrix( dev, "world_view_projection_xform", &world_view_projection_xform );

		// configure the pipeline - rasterizer
		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

		dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 
	
		// configure the pipeline - pixel shader
		IDirect3DPixelShader9 *_pixel_shader = _ambient->pixel();
		ID3DXConstantTable *_pixel_shader_constants = _ambient->pixel_constants();

		dev->SetPixelShader( _pixel_shader );
		_pixel_shader_constants->SetVector( dev, "camera_position", &camera->getPosition() );

		// configure the pipeline - framebuffer output

		// draw (execute the pipeline)
		dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, _num_vertices, 0, _num_triangles );
	}
}

void CEntity::draw( IDirect3DDevice9 *dev, CFirstPersonCamera *camera, CLight *light, IDirect3DTexture9 *shadow_map )
{
	D3DVIEWPORT9 viewport;
	dev->GetViewport( &viewport );

	D3DXMATRIX world_xform, rotation_x_xform, rotation_y_xform, rotation_z_xform, translation_xform;

	D3DXMatrixRotationX( &rotation_x_xform, _rotation.x );        // Pitch
	D3DXMatrixRotationY( &rotation_y_xform, _rotation.y );        // Yaw
	D3DXMatrixRotationZ( &rotation_z_xform, _rotation.z );        // Roll

	D3DXMatrixTranslation( &translation_xform, _position.x, _position.y, _position.z );

	world_xform = ( rotation_z_xform * rotation_x_xform * rotation_y_xform ) * translation_xform;

	// compute the view matrix using the camera object
	D3DXMATRIX view_xform = camera->ViewTransformation( );

	// compute the projection matrix using the camera object
	D3DXMATRIX projection_xform = camera->ProjectionTransformation( (float)viewport.Width / (float)viewport.Height );

	// combine the three pipeline matrices into a single matrix
	D3DXMATRIX world_view_projection_xform = world_xform * view_xform * projection_xform;

	// if shaders compiled correctly
	if ( _light->isCompiled() )
	{
		// configure the pipeline - primitive assembly
		dev->SetVertexDeclaration( _vertex_declaration );
		dev->SetStreamSource( 0, _vertex_buffer, 0, sizeof(Vertex) );
		dev->SetIndices( _index_buffer );

		// configure the pipeline - vertex shader

		IDirect3DVertexShader9 *_vertex_shader = _light->vertex();
		ID3DXConstantTable *_vertex_shader_constants = _light->vertex_constants();

		dev->SetVertexShader( _vertex_shader );
		_vertex_shader_constants->SetMatrix( dev, "world_xform", &world_xform );
		_vertex_shader_constants->SetMatrix( dev, "world_view_projection_xform", &world_view_projection_xform );

		// configure the pipeline - rasterizer
		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

		dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);

		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		// configure the pipeline - pixel shader
		IDirect3DPixelShader9 *_pixel_shader = _light->pixel();
		ID3DXConstantTable *_pixel_shader_constants = _light->pixel_constants();

		dev->SetPixelShader( _pixel_shader );
		_pixel_shader_constants->SetVector( dev, "camera_position", &camera->getPosition() );
		_pixel_shader_constants->SetVector( dev, "light_position", &light->getPosition() );
		_pixel_shader_constants->SetMatrix( dev, "light_view_projection_xform", &light->getViewProjection());
		_pixel_shader_constants->SetFloat( dev, "texture_size", MAP_SIZE );

		D3DXHANDLE hSpots = _pixel_shader_constants->GetConstantByName(0, "spot_light");
		float pos[3] = { light->getLight().position.x, light->getLight().position.y, light->getLight().position.z };
		float dir[3] = { light->getLight().direction.x, light->getLight().direction.y, light->getLight().direction.z };
		D3DXHANDLE hLight = _pixel_shader_constants->GetConstantElement(hSpots, 0);
		_pixel_shader_constants->SetFloatArray(dev, _pixel_shader_constants->GetConstantByName(hLight, "position"), pos, 3);
		_pixel_shader_constants->SetFloatArray(dev, _pixel_shader_constants->GetConstantByName(hLight, "direction"), dir, 3);
		_pixel_shader_constants->SetFloat(dev, _pixel_shader_constants->GetConstantByName(hLight, "cone_angle"), light->getLight().coneAngle * 2.0f);
		_pixel_shader_constants->SetFloat(dev, _pixel_shader_constants->GetConstantByName(hLight, "intensity"), light->getLight().intensity);

		dev->SetTexture( 0, shadow_map );
		dev->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
		dev->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );
		dev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		dev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		dev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

		// configure the pipeline - framebuffer output

		// draw (execute the pipeline)
		dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, _num_vertices, 0, _num_triangles );
	}
}

void CEntity::drawShadows( IDirect3DDevice9 *dev, CLight *light )
{
	D3DXMATRIX world_xform, rotation_x_xform, rotation_y_xform, rotation_z_xform, translation_xform;

	D3DXMatrixRotationX( &rotation_x_xform, _rotation.x );        // Pitch
	D3DXMatrixRotationY( &rotation_y_xform, _rotation.y );        // Yaw
	D3DXMatrixRotationZ( &rotation_z_xform, _rotation.z );        // Roll

	D3DXMatrixTranslation( &translation_xform, _position.x, _position.y, _position.z );

	world_xform = ( rotation_z_xform * rotation_x_xform * rotation_y_xform ) * translation_xform;

	// compute the view matrix using the camera object
	D3DXMATRIX view_projection_xform = light->getViewProjection();

	// combine the three pipeline matrices into a single matrix
	D3DXMATRIX world_view_projection_xform = world_xform * view_projection_xform;

	// if shaders compiled correctly
	if ( _shadow->isCompiled() )
	{
		// configure the pipeline - primitive assembly
		dev->SetVertexDeclaration( _vertex_declaration );
		dev->SetStreamSource( 0, _vertex_buffer, 0, sizeof(Vertex) );
		dev->SetIndices( _index_buffer );

		// configure the pipeline - vertex shader

		IDirect3DVertexShader9 *_vertex_shader = _shadow->vertex();
		ID3DXConstantTable *_vertex_shader_constants = _shadow->vertex_constants();

		dev->SetVertexShader( _vertex_shader );
		_vertex_shader_constants->SetMatrix( dev, "world_view_projection_xform", &world_view_projection_xform );

		// configure the pipeline - rasterizer
		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

		dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 

		// configure the pipeline - pixel shader
		IDirect3DPixelShader9 *_pixel_shader = _shadow->pixel();
		ID3DXConstantTable *_pixel_shader_constants = _shadow->pixel_constants();

		dev->SetPixelShader( _pixel_shader );

		// configure the pipeline - framebuffer output
		

		// draw (execute the pipeline)
		dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, _num_vertices, 0, _num_triangles );
	}
}

void CEntity::update( const Shape &shape )
{
	//update positions and rotation
	_position = D3DXVECTOR3( shape.position.x, shape.position.y, shape.position.z );
	_rotation = D3DXVECTOR3( shape.rotation.x, shape.rotation.y, shape.rotation.z );
}

void CEntity::setshaders( CShader *light, CShader *shadow, CShader *ambient )
{
	_light = light;
	_shadow = shadow;
	_ambient = ambient;
}
#include "CShader.h"

CShader::CShader()
{
	_vertex_shader = NULL;
	_vertex_shader_constants = NULL;
	_pixel_shader = NULL; 
	_pixel_shader_constants = NULL;
}

CShader::~CShader()
{
	Release( &_vertex_shader );
	Release( &_vertex_shader_constants );
	Release( &_pixel_shader );
	Release( &_pixel_shader_constants );
}

bool CShader::init( IDirect3DDevice9 *dev, char *vertex, char *pixel )
{
	#ifdef _DEBUG
		const DWORD shader_flags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	#else
		const DWORD shader_flags = 0;
	#endif

	ID3DXBuffer* shader = 0;
	ID3DXBuffer* errors = 0;

	if ( FAILED(D3DXCompileShaderFromFileA( vertex, 0, 0, "main", "vs_3_0", shader_flags, &shader, &errors, &_vertex_shader_constants )) )
	{
		if ( errors )
			std::cerr << "Failed to compile vertex shader: " << (const char*)errors->GetBufferPointer() << std::endl;
		else
			std::cerr << "Could not find vertex shader file - " << vertex << std::endl;

		return false;
	}
	else
		dev->CreateVertexShader( (const DWORD*)shader->GetBufferPointer(), &_vertex_shader );
	Release( &shader );
	Release( &errors );

	if ( FAILED(D3DXCompileShaderFromFileA( pixel, 0, 0, "main", "ps_3_0", shader_flags, &shader, &errors, &_pixel_shader_constants )) )
	{
		if ( errors )
			std::cerr << "Failed to compile pixel shader: " << (const char*)errors->GetBufferPointer() << std::endl;
		else
			std::cerr << "Could not find pixel shader file - " << pixel << std::endl;

		return false;
	}
	else
		dev->CreatePixelShader( (const DWORD*)shader->GetBufferPointer(), &_pixel_shader );
	Release( &shader );
	Release( &errors );

	return true;
}

void CShader::reload( IDirect3DDevice9 *dev, char *vertex, char *pixel )
{
	Release( &_vertex_shader );
	Release( &_vertex_shader_constants );
	Release( &_pixel_shader );
	Release( &_pixel_shader_constants );
	init( dev, vertex, pixel );
}

IDirect3DVertexShader9 *CShader::vertex()
{
	return _vertex_shader;
}

ID3DXConstantTable *CShader::vertex_constants()
{
	return _vertex_shader_constants;
}

IDirect3DPixelShader9 *CShader::pixel()
{
	return _pixel_shader;
}

ID3DXConstantTable *CShader::pixel_constants()
{
	return _pixel_shader_constants;
}

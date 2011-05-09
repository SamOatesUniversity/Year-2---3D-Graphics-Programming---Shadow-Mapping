//================================================================================================
// Name        : cast_a_shadow.cpp
// Author(s)   : Tyrone Davison
// Version     : 1.0
// Copyright   : Tyrone Davison, Teesside University, 2011
// Description : C++ framework for 3D Graphics Programming assignment
//================================================================================================

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>
#include "SceneDelegate.hpp"

#include "CEntity.h"
#include "CFirstPersonCamera.h"
#include "CLight.h"

class D3D9Window {
public:
	D3D9Window(void);
	~D3D9Window(void);
public:
	bool Init(unsigned int width, unsigned int height);
	void Run();
	void Deinit();

private:
	void CreateManagedResources();
	void DestroyManagedResources();
	void CreateUnmanagedResources();
	void DestroyUnmanagedResources();
	void UpdateFrame(float time);
	void DrawFrame();

private:
	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	//////////////////////////////
	// Template Function		//
	// Release is for Direct3D  //
	//////////////////////////////
	template<typename T>
	void Release(T** ptr) {
		if ( *ptr != 0 ) {
			(*ptr)->Release();
			*ptr = 0;
		}
	}
	//////////////////////////////////////////////////
	// Template Function							//
	// Release is for standered new/delete objects  //
	//////////////////////////////////////////////////
	template<typename T>
	void Free(T** ptr) {
		if ( *ptr != 0 ) {
			delete (*ptr);
			*ptr = 0;
		}
	}

private: // members for basic windowing
	HWND _wnd; // window handle
	bool _run; // flag to indicate when the message loop should exit
private: // members for basic D3D9
	IDirect3D9* _d3d; // Direct3D object
	IDirect3DDevice9* _dev; // Direct3D rendering device
	bool _lost; // flag to indicate the device needs resetting
	D3DPRESENT_PARAMETERS _pp; // device settings needed for resetting it when it is lost
private:
	LPDIRECT3DTEXTURE9  _shadow_texture; //Texture to store shadow map in
	IDirect3DSurface9* _shadow_rendertarget; //The first surface level of the shadow texture
	IDirect3DSurface9* _shadow_depthstencil; //The depth stencil for the shadow pass

	IDirect3DSurface9* _window_rendertarget; //The default screen render target
	IDirect3DSurface9* _window_depthstencil; //The default septh stencil

	SceneDelegate* _scene_delegate; //The sceene delegate stores all the sceenes information.
									//This includes meshes and lighting

	std::vector<CEntity*> _entity; //Store all geometry objects

	CFirstPersonCamera *_camera;	//A first person camera used to navigate the sceene
	CFirstPersonCamera *getCamera() { return _camera; } //A simple getter for the camera

	void reloadShaders( void );	//Reloads all three shaders (pixel and vertex)
	CShader *_light, *_shadow, *_ambient; //The free shaders used in the sceene
};

//Initilise unmanaged resources to null
D3D9Window::D3D9Window() : 
	_wnd(0), _run(true),
	_d3d(0), _dev(0), _lost(true), 
	_window_rendertarget(0), _window_depthstencil(0), _shadow_rendertarget(0),  
	_shadow_depthstencil(0), _shadow_texture(0)
{
	//Create an instance of the sceene delegate
	_scene_delegate = new SceneDelegate();
	//Create an instance of the camera
	_camera = new CFirstPersonCamera();
}

D3D9Window::~D3D9Window() {
	assert("D3D9Window::Shutdown not performed" && _wnd == 0);
	delete _scene_delegate;
}

bool D3D9Window::Init(unsigned int width, unsigned int height) {
	assert( "D3D9Window::Init already performed" && _wnd == 0 );

	// Define a window "class" - this is Win32 not D3D9
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = 0;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = 0;
	wc.hInstance = 0;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "3GP";
	wc.lpszMenuName = 0;
	wc.style = CS_OWNDC;
	if (0 == RegisterClass(&wc)) {
		std::cerr << "D3D9Window::Init failed; RegisterClass failed" << std::endl;
		return false;
	}

	// Adjust the window size so the client area matches the requested size
	RECT client;
	client.left = client.top = 0;
	client.right = width;
	client.bottom = height;
	AdjustWindowRect(&client, WS_OVERLAPPEDWINDOW, FALSE);

	// Create a window of your window "class" - this is Win32 not D3D9
	_wnd = CreateWindow("3GP",
		"3D Graphics Programming Framework, Tyrone Davison, Teesside University",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		client.right - client.left, client.bottom - client.top, 0, 0, 0, 0);
	if (_wnd == 0) {
		std::cerr << "D3D9Window::Init failed; CreateWindow failed" << std::endl;
		return false;
	}

	// Store a pointer to this object in the window - this is helpful for later
	SetWindowLongPtr(_wnd, GWL_USERDATA, (LONG_PTR)this);

	// Create Direct3D - this isn't the device yet
	_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (_d3d == 0) {
		std::cerr << "D3D9Window::Init failed; Direct3DCreate9 failed" << std::endl;
		DestroyWindow(_wnd);
		_wnd = 0;
		return false;
	}

	// Check for multisample support
	D3DDISPLAYMODE display_mode;
	_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode);
	D3DMULTISAMPLE_TYPE multisample_type = D3DMULTISAMPLE_NONE;
	if (SUCCEEDED(_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		display_mode.Format, TRUE,
		D3DMULTISAMPLE_4_SAMPLES, 0))) {
			multisample_type = D3DMULTISAMPLE_4_SAMPLES;
	}

	// Configure the device settings
	_pp.BackBufferWidth = 0;
	_pp.BackBufferHeight = 0;
	_pp.BackBufferFormat = D3DFMT_UNKNOWN;
	_pp.BackBufferCount = 1;
	_pp.MultiSampleType = multisample_type;
	_pp.MultiSampleQuality = 0;
	_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_pp.hDeviceWindow = _wnd;
	_pp.Windowed = TRUE;
	_pp.EnableAutoDepthStencil = TRUE;
	_pp.AutoDepthStencilFormat = D3DFMT_D24X8;
	_pp.Flags = 0;
	_pp.FullScreen_RefreshRateInHz = 0;
	_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create a rendering device
	if (FAILED(_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _wnd,
		D3DCREATE_MIXED_VERTEXPROCESSING, &_pp, &_dev))) {
			std::cerr << "D3D9Window::Init failed; IDirect3D9::CreateDevice failed" << std::endl;
			Deinit();
			return false;
	}

	// Allocate resources
	CreateManagedResources();

	return true;
}

void D3D9Window::Run() {
	assert("D3D9Window::Init not performed" && _wnd != 0);

	// Make the window appear - this is Win32 not D3D9
	ShowWindow(_wnd, SW_SHOW);

	// Establish a "message loop", continuously listening for and processing window events,
	//   updating and drawing the "scene"
	MSG msg;
	timeBeginPeriod(1);
	const long ideal_frame_time = 16;
	long start_time = timeGetTime();
	while (_run == true) {

		// Handle all pending events - this is Win32 not D3D9
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If the device has just been lost, prepare for reset
		if (_lost == false && _dev->TestCooperativeLevel() != D3D_OK) {
			_lost = true;
			DestroyUnmanagedResources();
		}

		// If device is lost, attempt to reset it
		if (_lost == true && _pp.BackBufferWidth != 0 && _pp.BackBufferHeight != 0
			&& SUCCEEDED(_dev->Reset(&_pp))) {
				_lost = false;
				CreateUnmanagedResources();
		}

		// If device is operational, draw
		if (_lost == false) {

			// Update the scene
			UpdateFrame(start_time * 0.001f);

			// Render the scene
			DrawFrame();
		}

		// Regulate the frame rate
		long delta_time = timeGetTime() - start_time;
		if (delta_time < ideal_frame_time) {
			Sleep(ideal_frame_time - delta_time - 1);
			start_time += ideal_frame_time;
		}
		else {
			start_time = timeGetTime();
		}
	}
	timeEndPeriod(1);
}

void D3D9Window::Deinit()
{
	assert("D3D9Window::Init not performed" && _wnd != 0);

	// Release resources
	DestroyManagedResources();
	DestroyUnmanagedResources();

	// Release the Direct3D primary interfaces
	Release(&_dev);
	Release(&_d3d);

	// Destroy the window - this is Win32 not D3D9
	DestroyWindow(_wnd);
	_wnd = 0;
}

//Release the currently store shaders,
//Reload them. Allows shaders
void D3D9Window::reloadShaders( void )
{
	std::cout<< "Reloading Shaders\n";
	_light->reload( _dev, "Lighting.vsh", "Lighting.psh" );
	_shadow->reload( _dev, "Shadow.vsh", "Shadow.psh" );
	_ambient->reload( _dev, "Ambient.vsh", "Ambient.psh" );
}

//Create managed resources
void D3D9Window::CreateManagedResources() {
	
	//Initialise camera, position and rotate it
	D3DXVECTOR3 camera_pos = D3DXVECTOR3( 16.5f, -21.0f, 11.5f );
	D3DXVECTOR3 camera_rotation = D3DXVECTOR3( -0.5f, 3.71f, 0.0f );
	_camera->init( _scene_delegate, camera_pos, camera_rotation );

	//Load the lighting shader
	_light = new CShader();
	if(	!_light->init( _dev, "Lighting.vsh", "Lighting.psh" ) )
	{
		//if it failed to load quit
		_run = false;
	}

	//Load the shadow shader
	_shadow = new CShader();
	if(	!_shadow->init( _dev, "Shadow.vsh", "Shadow.psh" ) )
	{
		//if it failed to load quit
		_run = false;
	}

	//Load the ambient shader
	_ambient = new CShader();
	if(	!_ambient->init( _dev, "Ambient.vsh", "Ambient.psh" ) )
	{
		//if it failed to load quit
		_run = false;
	}

	//Create entities based upon the meshes from the sceene delegate
	for( UINT i = 0; i < _scene_delegate->numberOfShapes(); i++ )
	{
		Mesh mesh;
		_scene_delegate->getMeshAtIndex( i, &mesh );
		Shape shape = _scene_delegate->shapeAtIndex( i );

		CEntity *new_ent = new CEntity();
		new_ent->init( _dev, &mesh, shape );
		new_ent->setshaders( _light, _shadow, _ambient );
		_entity.push_back( new_ent );
	}

}

void D3D9Window::DestroyManagedResources() 
{
	//Release allocated memory
	Free( &_camera );
	Free( &_light );
	Free( &_shadow );
	Free( &_ambient );

	for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
		Free( &(*ent) );

	_entity.clear();
}

void D3D9Window::CreateUnmanagedResources() 
{
	// retrieve pointers to the backbuffer surfaces
	_dev->GetRenderTarget(0, &_window_rendertarget);
	_dev->GetDepthStencilSurface( &_window_depthstencil );

	//Create depth stencil for shadow pass
	_dev->CreateDepthStencilSurface( MAP_SIZE, MAP_SIZE,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE,
		0, TRUE,
		&_shadow_depthstencil,
		NULL );


	//Create the texture to render the shadow pass too
	//Format is D3DFMT_G32R32F as two channels are needed for Varience Shadow mapping
    if( FAILED(  _dev->CreateTexture( MAP_SIZE, MAP_SIZE, 0, D3DUSAGE_RENDERTARGET,
                                    D3DFMT_G32R32F, D3DPOOL_DEFAULT, &_shadow_texture,
                                    NULL ) ) )
	{
		std::cout << "Error - Could not create shadow map texture\n";   
		_run = false;
	}

	//grab its surface
	_shadow_texture->GetSurfaceLevel( 0, &_shadow_rendertarget );

}

void D3D9Window::DestroyUnmanagedResources() {
	// release backbuffer surfaces before resize
	Release( &_window_rendertarget );
	Release( &_window_depthstencil );
	Release( &_shadow_rendertarget );
	Release( &_shadow_depthstencil );
	Release( &_shadow_texture );
}

void D3D9Window::UpdateFrame(float time) {
	_scene_delegate->animate(time);

	//update all entities with updates sceene delegate information
	for( UINT i = 0; i < _scene_delegate->numberOfShapes(); i++ ) 
	{
		_entity[i]->update( _scene_delegate->shapeAtIndex( i ) );
	}
}

void D3D9Window::DrawFrame() {

	//Draw the sceene with ambient lighting
	_dev->BeginScene();

	_dev->SetRenderTarget( 0, _window_rendertarget );
	_dev->SetDepthStencilSurface( _window_depthstencil );
	_dev->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,64), 1.0f, 0 );

	for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
	{
		(*ent)->drawAmbient( _dev, _camera );
	}

	_dev->EndScene();

	//For each light source in the sceene
	for( UINT l = 0; l < _scene_delegate->numberOfLights(); l++ )
	{
		CLight light( _scene_delegate, l );

		//Draw the shadows from the lights perspective
		_dev->BeginScene();
		_dev->SetRenderTarget( 0, _shadow_rendertarget );
		_dev->SetDepthStencilSurface( _shadow_depthstencil );
		_dev->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,64), 1.0f, 0 );

		for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
		{
			(*ent)->drawShadows( _dev, &light );
		}
		_dev->EndScene();

		//Draw the sceene normally
		_dev->BeginScene();
		_dev->SetRenderTarget(0, _window_rendertarget);
		_dev->SetDepthStencilSurface( _window_depthstencil );

		for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
		{
			(*ent)->draw( _dev, _camera, &light, _shadow_texture );
		}
		_dev->EndScene();
	}

	_dev->Present(0, 0, 0, 0);

}

LRESULT CALLBACK D3D9Window::WndProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	// Retrieve a pointer to the D3D9Window object
	D3D9Window* window = (D3D9Window*)GetWindowLongPtr( wnd, GWL_USERDATA );
	if ( window == 0 )
		return DefWindowProc( wnd, msg, wParam, lParam );

	// Handle required messages
	switch (msg) {
	case WM_KEYDOWN:
		switch ( wParam )
		{
		case VK_F5:
			window->reloadShaders();
			break;
		case VK_LEFT:
			window->getCamera()->move( D3DXVECTOR3( -2.0f, 0.0f, 0.0f ) );
			break;
		case VK_RIGHT:
			window->getCamera()->move( D3DXVECTOR3( 2.0f, 0.0f, 0.0f ) );
			break;
		case VK_UP:
			window->getCamera()->move( D3DXVECTOR3( 0.0f, 0.0f, 2.0f ) );
			break;
		case VK_DOWN:
			window->getCamera()->move( D3DXVECTOR3( 0.0f, 0.0f, -2.0f ) );
			break;
		case 'A':
			window->getCamera()->move( D3DXVECTOR3( -2.0f, 0.0f, 0.0f ) );
			break;
		case 'D':
			window->getCamera()->move( D3DXVECTOR3( 2.0f, 0.0f, 0.0f ) );
			break;
		case 'W':
			window->getCamera()->move( D3DXVECTOR3( 0.0f, 0.0f, 2.0f ) );
			break;
		case 'S':
			window->getCamera()->move( D3DXVECTOR3( 0.0f, 0.0f, -2.0f ) );
			break;
		}
		break;
	case WM_MOUSEMOVE:
		switch ( wParam )
		{
		case MK_LBUTTON:
			window->getCamera()->turn( D3DXVECTOR3( 
				(window->getCamera()->getLastMouseXY().y - HIWORD( lParam )) * 0.01f, 
				(window->getCamera()->getLastMouseXY().x - LOWORD( lParam )) * 0.01f, 
				0.0f ) );
			break;
		}
		window->getCamera()->setLastMouseXY( LOWORD( lParam ), HIWORD( lParam ) );
		break;
	case WM_CLOSE:
		ShowWindow(window->_wnd, SW_HIDE);
		window->_run = false;
		break;
	case WM_SIZE:
		// Simulate a lost device so the reset will resize the framebuffers
		if (window->_lost == false) {
			window->_lost = true;
			window->DestroyUnmanagedResources();
		}
		// Store the new window size in the device settings ready for the reset
		window->_pp.BackBufferWidth = LOWORD(lParam);
		window->_pp.BackBufferHeight = HIWORD(lParam);
		break;
	default:
		return DefWindowProc(wnd, msg, wParam, lParam);
	}

	return 0;
}

int main(int argc, char* argv[]) {
	D3D9Window* window = new D3D9Window();
	if (window->Init(1024, 576)) {
		window->Run();
		window->Deinit();
	}
	delete window;

	// Pause to display any console messages
	system("PAUSE");
	return 0;
}

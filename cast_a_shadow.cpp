//================================================================================================
// Name        : cast_a_shadow.cpp
// Author(s)   : Tyrone Davison
// Version     : 1.0
// Copyright   : Tyrone Davison, Teesside University, 2011
// Description : C++ framework for 3D Graphics Programming assignment
//================================================================================================

#define RELOAD_SHADOWS_TIMER		101010
#define TYRONE_RUNNING				FALSE

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <cassert>
#include "SceneDelegate.hpp"

#include "CEntity.h"
#include "CLookAtCamera.h"
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
	template<typename T>
	void Release(T** ptr) {
		if ( *ptr != 0 ) {
			(*ptr)->Release();
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
	LPDIRECT3DTEXTURE9  _shadow_texture;

	IDirect3DSurface9* _shadow_rendertarget;
	IDirect3DSurface9* _shadow_depthstencil;

	IDirect3DSurface9* _window_rendertarget;
	IDirect3DSurface9* _window_depthstencil;

	SceneDelegate* _scene_delegate;

	std::vector<CEntity*> _entity; //to store all geometry objects in

	CFirstPersonCamera *_camera;
	CFirstPersonCamera *getCamera() { return _camera; }

	void reloadShadows( void );
	CShader *_light, *_shadow, *_ambient;
};

D3D9Window::D3D9Window() : _wnd(0), _run(false),
	_d3d(0), _dev(0), _lost(true), // device is lost from the start
	_window_rendertarget(0), _window_depthstencil(0) {
		_scene_delegate = new SceneDelegate();
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

	//Create a timer if in debug mode, for auto reloading of shaders
#if !TYRONE_RUNNING
	SetTimer(_wnd,             // handle to main window 
		RELOAD_SHADOWS_TIMER,            // timer identifier 
		3000,                 // 10-second interval 
		(TIMERPROC) NULL);     // no timer callback 
#endif


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
	_run = true;
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

void D3D9Window::reloadShadows( void )
{
	std::cout<< "Reloading Shaders\n";
	_light->reload( _dev, "Lighting.vsh", "Lighting.psh" );
	_shadow->reload( _dev, "Shadow.vsh", "Shadow.psh" );
	_ambient->reload( _dev, "Ambient.vsh", "Ambient.psh" );
}

void D3D9Window::CreateManagedResources() {
	
	_camera->init( 16.5f, -21.0f, 11.5f );
	_camera->setRotation( D3DXVECTOR3( -0.36f, 3.61f, 0.0f ) );

	_light = new CShader();
	_light->init( _dev, "Lighting.vsh", "Lighting.psh" );

	_shadow = new CShader();
	_shadow->init( _dev, "Shadow.vsh", "Shadow.psh" );

	_ambient = new CShader();
	_ambient->init( _dev, "Ambient.vsh", "Ambient.psh" );

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

void D3D9Window::DestroyManagedResources() {

	delete _camera;
	delete _light;
	delete _shadow;
	delete _ambient;
	for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
		delete (*ent);
}

void D3D9Window::CreateUnmanagedResources() {
	// retrieve pointers to the backbuffer surfaces
	_dev->GetRenderTarget(0, &_window_rendertarget);
	_dev->GetDepthStencilSurface( &_window_depthstencil );

	_dev->CreateDepthStencilSurface( MAP_SIZE, MAP_SIZE,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE,
		0, TRUE,
		&_shadow_depthstencil,
		NULL );

    if( FAILED(  _dev->CreateTexture( MAP_SIZE, MAP_SIZE, 0, D3DUSAGE_RENDERTARGET,
                                    D3DFMT_G32R32F, D3DPOOL_DEFAULT, &_shadow_texture,
                                    NULL ) ) )
	{
		MessageBox(NULL, "Could not create shadow map texture", "", MB_OK | MB_ICONERROR );   
	}

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

	for( UINT i = 0; i < _scene_delegate->numberOfShapes(); i++ ) 
	{
		_entity[i]->update( _scene_delegate->shapeAtIndex( i ) );
	}
}

void D3D9Window::DrawFrame() {

	_dev->BeginScene();

	_dev->SetRenderTarget( 0, _window_rendertarget );
	_dev->SetDepthStencilSurface( _window_depthstencil );
	_dev->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,64), 1.0f, 0 );

	for( std::vector<CEntity*>::iterator ent = _entity.begin(); ent != _entity.end(); ++ent ) 
	{
		(*ent)->drawAmbient( _dev, _camera );
	}

	_dev->EndScene();

	for( UINT l = 0; l < _scene_delegate->numberOfLights(); l++ )
	{
		CLight light( _scene_delegate->lightAtIndex( l ) );

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
#if !TYRONE_RUNNING
	case WM_TIMER:
		switch( wParam )
		{
		case RELOAD_SHADOWS_TIMER:
			window->reloadShadows();
			break;
		}
		break;
#endif
	case WM_KEYDOWN:
		switch ( wParam )
		{
		case VK_F5:
			window->reloadShadows();
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

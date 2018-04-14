// CGS HW Project A "Line Land".
// Author: L.Norri CD CGS, FullSail University

// Introduction:
// Welcome to the hardware project of the Computer Graphics Systems class.
// This assignment is fully guided but still requires significant effort on your part. 
// Future classes will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this project and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"

#include "greendragon.h"
#define DRAGON greendragon_pixels

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>
#pragma comment(lib, "d3d11");

// TODO: PART 1 STEP 1b
#include <DirectXMath.h>

// TODO: PART 2 STEP 6
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	// TODO: PART 1 STEP 2
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* dsv;
	D3D11_VIEWPORT viewport;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IDXGISwapChain* swapChain;

	// TODO: PART 2 STEP 2
	ID3D11Buffer* buffer;
	unsigned int vertCount;
	ID3D11InputLayout* inputLayout;

	ID3D11Buffer* indexBuffer;

	//D3D11_INPUT_ELEMENT_DESC vLayout[];

		// BEGIN PART 5
		// TODO: PART 5 STEP 1
	ID3D11Buffer* gridBuffer;
	unsigned int gridVertCount = 0;

		// TODO: PART 2 STEP 4
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	// BEGIN PART 3
	// TODO: PART 3 STEP 1
	ID3D11Buffer* cBuffer;
	D3D11_BUFFER_DESC cBufferDesc = {};
	XTime time;
	float timer = 0.0;
	float cameraTimer = 0.0;

	DirectX::XMMATRIX worldMatrix1;
	DirectX::XMMATRIX worldMatrix2;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX cameraMatrix;
	DirectX::XMMATRIX projectionMatrix;

	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMFLOAT4 light;
		//DirectX::XMFLOAT4 constantColor;

		//DirectX::XMFLOAT2 constantOffset;
		//DirectX::XMFLOAT2 padding;
	};

	struct SEND_TO_PS
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 lightDirection;
		float padding;
	};

	struct SCENE_TO_SEND
	{
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMFLOAT4X4 projectionMatrix;
	};

	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;
	SCENE_TO_SEND toSceneShader;

	SEND_TO_VRAM toGridShader;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT4 pos;
		//DirectX::XMFLOAT2 texCoords;
		DirectX::XMFLOAT4 color;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run(); 
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	// TODO: PART 1 STEP 3a
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Width = 500;
	swapChainDesc.BufferDesc.Height = 500;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	// TODO: PART 1 STEP 3b
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &device, NULL, &context);

	// TODO: PART 1 STEP 4
	ID3D11Texture2D* backBuffer;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();

	ID3D11Texture2D* depthBuffer;

	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = BACKBUFFER_WIDTH;
	depthDesc.Height = BACKBUFFER_HEIGHT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	device->CreateTexture2D(&depthDesc, NULL, &depthBuffer);

	device->CreateDepthStencilView(depthBuffer, NULL, &dsv);

	// TODO: PART 1 STEP 5
	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// TODO: PART 2 STEP 3a
	SIMPLE_VERTEX cubeVerts[8];

#if 1
	//Bottom Left, -z
	cubeVerts[0].pos.x = -0.25f;
	cubeVerts[0].pos.y = -0.25f;
	cubeVerts[0].pos.z = -0.25f;
	cubeVerts[0].pos.w = 1;
	cubeVerts[0].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	//Bottom Left, +z
	cubeVerts[1].pos.x = -0.25f;
	cubeVerts[1].pos.y = -0.25f;
	cubeVerts[1].pos.z = 0.25f;
	cubeVerts[1].pos.w = 1;
	cubeVerts[1].color = { 1.0f, 0.0f, 1.0f, 1.0f };
	//Bottom Right, +z
	cubeVerts[2].pos.x = 0.25f;
	cubeVerts[2].pos.y = -0.25f;
	cubeVerts[2].pos.z = 0.25f;
	cubeVerts[2].pos.w = 1;
	cubeVerts[2].color = { 1.0f, 1.0f, 0.0f, 1.0f };
	//Bottom Right, -z
	cubeVerts[3].pos.x = 0.25f;
	cubeVerts[3].pos.y = -0.25f;
	cubeVerts[3].pos.z = -0.25f;
	cubeVerts[3].pos.w = 1;
	cubeVerts[3].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	//Top Left, -z
	cubeVerts[4].pos.x = -0.25f;
	cubeVerts[4].pos.y = 0.25f;
	cubeVerts[4].pos.z = -0.25f;
	cubeVerts[4].pos.w = 1;
	cubeVerts[4].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	//Top Left, +z
	cubeVerts[5].pos.x = -0.25f;
	cubeVerts[5].pos.y = 0.25f;
	cubeVerts[5].pos.z = 0.25f;
	cubeVerts[5].pos.w = 1;
	cubeVerts[5].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//Top Right, +z
	cubeVerts[6].pos.x = 0.25f;
	cubeVerts[6].pos.y = 0.25f;
	cubeVerts[6].pos.z = 0.25f;
	cubeVerts[6].pos.w = 1;
	cubeVerts[6].color = { 0.0f, 1.0f, 1.0f, 1.0f };
	//Top Right, -z
	cubeVerts[7].pos.x = 0.25f;
	cubeVerts[7].pos.y = 0.25f;
	cubeVerts[7].pos.z = -0.25f;
	cubeVerts[7].pos.w = 1;
	cubeVerts[7].color = { 0.0f, 0.0f, 0.0f, 1.0f };

	short triangleVerts[] = {
	//Front Face
		0,4,3, 4,7,3,
	//Right Face
		3,7,2, 7,6,2,
	//Back Face
		2,6,1, 6,5,1,
	//Left Face
		1,5,0, 5,4,0,
	//Top Face
		4,5,7, 5,6,7,
	//Bottom Face
		1,0,2, 0,3,2
	};
	
#endif

	// BEGIN PART 4
	// TODO: PART 4 STEP 1
	/*for (unsigned int i = 0; i <= 7; ++i)
	{
		circleVerts[i].pos.x *= 0.2f;
		circleVerts[i].pos.y *= 0.2f;
	}*/

	// TODO: PART 2 STEP 3b
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 36;

	// TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA bufferResource = {};
	bufferResource.pSysMem = cubeVerts;

	device->CreateBuffer(&bufferDesc, &bufferResource, &buffer);

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = NULL;
	indexDesc.ByteWidth = sizeof(short) * 36;

	// TODO: PART 2 STEP 3d
	bufferResource.pSysMem = triangleVerts;

	device->CreateBuffer(&indexDesc, &bufferResource, &indexBuffer);

	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_SUBRESOURCE_DATA texSrc[greendragon_numlevels];
	ZeroMemory(&texDesc, sizeof(texDesc));

	for (int i = 0; i < greendragon_numlevels; ++i)
	{
		ZeroMemory(&texSrc[i], sizeof(texSrc[i]));
		texSrc[i].pSysMem = &DRAGON[greendragon_leveloffsets[i]];
		texSrc[i].SysMemPitch = (greendragon_width >> i) * sizeof(unsigned int);
	}

	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	texDesc.Height = greendragon_height;
	texDesc.Width = greendragon_width;
	texDesc.MipLevels = greendragon_numlevels;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.SampleDesc.Count = 1;

	HRESULT result = device->CreateTexture2D(&texDesc, texSrc, &diffuseTexture);

	// TODO: PART 5 STEP 2a

	// TODO: PART 5 STEP 2b
	
	// TODO: PART 5 STEP 3

	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	// TODO: PART 2 STEP 8a
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{ "TEXTURECOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};



	// TODO: PART 2 STEP 8b
	device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	// TODO: PART 3 STEP 3
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.ByteWidth = sizeof(SEND_TO_VRAM);

	device->CreateBuffer(&cBufferDesc, NULL, &cBuffer);

	// TODO: PART 3 STEP 4b
	worldMatrix1 = DirectX::XMMatrixIdentity();
	worldMatrix2 = DirectX::XMMatrixIdentity();
	cameraMatrix = DirectX::XMMatrixRotationX(-18.0f);
	cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -1.0f), cameraMatrix);
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(90, BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT, 0.1f, 10.0f);
	//toShader.constantOffset = { 0, 0 };
	//toShader.worldMatrix = DirectX::XMMatrixTranslation(0, 0.25f, 0);
	//toShader.viewMatrix = DirectX::XMMatrixRotationX(-18.0f);
	//toShader.viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -1.0f), toShader.viewMatrix);
	//toShader.viewMatrix = DirectX::XMMatrixInverse(nullptr, toShader.viewMatrix);
	//toShader.projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(90, 500 / 500, 0.1f, 10.0f);
	
	//toShader.light = { -0.333f, -0.333f, 0.333f, 0 };
	
	//toShader.constantColor = { 1.0f, 1.0f, 0.0f, 1.0f };

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	time.Signal();

	// TODO: PART 4 STEP 3
	
	// TODO: PART 4 STEP 5

	if (cameraTimer >= 0.025f)
	{
		if (GetAsyncKeyState(0x57)) // 'W'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.05f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x41)) // 'A'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(-0.05f, 0.0f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x53)) // 'S'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.05f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x44)) // 'D'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.05f, 0.0f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(VK_UP)) // UP ARROW KEY
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(VK_DOWN)) // DOWN ARROW KEY
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, -0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			cameraTimer = 0.0f;
		}
	}
	else
	{
		cameraTimer += time.SmoothDelta();
	}
	// END PART 4

	// TODO: PART 1 STEP 7a
	context->OMSetRenderTargets(1, &rtv, dsv);

	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);

	// TODO: PART 1 STEP 7c
	FLOAT arr[4] = { 0.0f, 0.0f, 0.7f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	// TODO: PART 5 STEP 4
	//toGridShader.constantColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	//toGridShader.constantOffset = { 0,0 };
	//toGridShader.padding = { 0.0f, 0.0f };

	// TODO: PART 5 STEP 5

	// TODO: PART 5 STEP 6

	// TODO: PART 5 STEP 7

	// END PART 5

	// TODO: PART 3 STEP 5
	worldMatrix1 = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(-timer), DirectX::XMMatrixTranslation(0.0f, 0.25f, 0.0f));
	toShader.worldMatrix = worldMatrix1;
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	toShader.viewMatrix = viewMatrix;
	toShader.projectionMatrix = projectionMatrix;

	timer += time.SmoothDelta();

	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toShader, sizeof(toShader));
	context->Unmap(cBuffer, NULL);

	// TODO: PART 3 STEP 6
	context->VSSetConstantBuffers(0, 1, &cBuffer);

	// TODO: PART 2 STEP 9a
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, offset);

	// TODO: PART 2 STEP 9b
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	// TODO: PART 2 STEP 9c
	context->IASetInputLayout(inputLayout);

	// TODO: PART 2 STEP 9d
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// TODO: PART 2 STEP 10
	context->DrawIndexed(36, 0, 0);

	worldMatrix2 = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(2.0f, 0.25f, 0.0f), DirectX::XMMatrixRotationY(timer * 0.8f));
	worldMatrix2 = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(timer * 0.2f), DirectX::XMMatrixRotationY(timer * 1.5f)), DirectX::XMMatrixRotationZ(timer * 0.8f)), worldMatrix2);
	worldMatrix2 = DirectX::XMMatrixMultiply(worldMatrix2, DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f));
	toShader.worldMatrix = worldMatrix2;

	D3D11_MAPPED_SUBRESOURCE ms2;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);
	memcpy(ms2.pData, &toShader, sizeof(toShader));
	context->Unmap(cBuffer, NULL);

	context->DrawIndexed(36, 0, 0);
	// END PART 2

	// TODO: PART 1 STEP 8
	swapChain->Present(0, 0);

	// END OF PART 1
	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6
	device->Release();
	swapChain->Release();
	rtv->Release();
	context->Release();
	cBuffer->Release();
	gridBuffer->Release();
	buffer->Release();
	ps->Release();
	vs->Release();
	inputLayout->Release();

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY): { PostQuitMessage(0); }
					   break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//
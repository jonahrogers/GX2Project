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

#include "DDSTextureLoader.h"

#include "Barrel.h"
#include "greendragon.h"
#include "StoneHenge.h"
#include "3d_planet.h"

#define TID_POLLMOUSE 100
#define MOUSE_POLL_DELAY 5

#define NUM_OF_MATRICIES 9
#define NUM_OF_VBUFFERS 5
#define NUM_OF_IBUFFERS 5
#define NUM_OF_DVIEWS 8
#define NUM_OF_SSTATES 1

using namespace std;

#include <d3d11.h>
#pragma comment(lib, "d3d11");

#include <DirectXMath.h>

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "Trivial_GS.csh"
#include "SkyBox_PS.csh"
#include "SkyBox_VS.csh"
#include "RTT_PS.csh"

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	ID3D11Device* device;
	ID3D11DeviceContext* context;
	ID3D11RenderTargetView* rtv;

	ID3D11RenderTargetView* rtt_rtv;
	ID3D11DepthStencilView* rtt_dsv;
	ID3D11Texture2D* rtt_texture;
	ID3D11Texture2D* rtt_depthBuffer;
	ID3D11ShaderResourceView* rtt_shaderResource;
	DirectX::XMMATRIX rtt_viewMatrix;
	DirectX::XMMATRIX rtt_projectionMatrix;

	ID3D11ShaderResourceView* const pSRV[1] = { NULL };

	ID3D11DepthStencilView* dsv;
	D3D11_VIEWPORT viewport;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* backBuffer;
	ID3D11Texture2D* depthBuffer;

	float backBuffer_width = 500;
	float backBuffer_height = 500;
	float fov = 90;

	ID3D11Buffer* vBuffers[NUM_OF_VBUFFERS];
	ID3D11Buffer* iBuffers[NUM_OF_IBUFFERS];
	ID3D11ShaderResourceView* diffuseViews[NUM_OF_DVIEWS];
	ID3D11SamplerState* samplerStates[NUM_OF_SSTATES];

	ID3D11InputLayout* inputLayout;
	ID3D11InputLayout* skyBox_inputLayout;

	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;
	ID3D11GeometryShader* gs;
	ID3D11VertexShader* skyBox_vs;
	ID3D11PixelShader* skyBox_ps;
	ID3D11PixelShader* rtt_ps;

	ID3D11Buffer* cBufferVS;
	D3D11_BUFFER_DESC cBufferVSDesc = {};
	ID3D11Buffer* cBufferPS;
	D3D11_BUFFER_DESC cBufferPSDesc = {};
	XTime time;
	float timer = 0.0;
	float cameraTimer = 0.0;

	DirectX::XMMATRIX worldMatricies[NUM_OF_MATRICIES];
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX cameraMatrix;
	POINT tempPointDown = { MININT, MININT };
	DirectX::XMMATRIX projectionMatrix;

	struct LIGHTS
	{
		DirectX::XMFLOAT3 lightDirection;
		float padding;
		DirectX::XMFLOAT4 lightPosition;
		DirectX::XMFLOAT4 lightColor;
	};

	struct SEND_TO_VS
	{
		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
	};

	struct SEND_TO_SKYBOX_VS
	{
		DirectX::XMMATRIX worldMatrix;
	};

	struct SEND_TO_PS
	{
		LIGHTS lights[3];
		DirectX::XMFLOAT4 ambientColor;
		//DirectX::XMFLOAT4 emissiveValue;
	};

	SEND_TO_VS toVSShader;
	SEND_TO_PS toPSShader;

public:
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoords;
	};

	struct SKYBOX_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoords;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run(); 
	bool ShutDown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

	void CheckCameraInput();
	bool LoadObjHeader(const OBJ_VERT* data, int dataSize, const unsigned int* indicies, int indiciesSize, int vBuffer, int iBuffer);
	void MakeVertexBuffer(ID3D11Buffer** buffer, int size, SIMPLE_VERTEX* data);
	void MakeVertexBuffer(ID3D11Buffer** buffer, int size, SKYBOX_VERTEX* data);
	void MakeIndexBuffer(ID3D11Buffer** buffer, int size, short* data);
};

DEMO_APP* myApp = nullptr;

void DEMO_APP::CheckCameraInput()
{
	if (cameraTimer >= 0.025f)
	{
		if (GetAsyncKeyState(0x57)) // 'W'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.05f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x41)) // 'A'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(-0.05f, 0.0f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x53)) // 'S'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.05f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x44)) // 'D'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.05f, 0.0f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(tempM, cameraMatrix);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x51)) // 'Q'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(cameraMatrix, tempM);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x45)) // 'E'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, -0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(cameraMatrix, tempM);

			worldMatricies[3].r[3] = cameraMatrix.r[3];

			cameraTimer = 0.0f;
		}
	}
	else
	{
		cameraTimer += time.SmoothDelta();
	}
}

bool DEMO_APP::LoadObjHeader(const OBJ_VERT* data, int dataSize, const unsigned int* indicies, int indiciesSize, int vBuffer, int iBuffer)
{
	SIMPLE_VERTEX* tempModel = new SIMPLE_VERTEX[dataSize];

	for (int i = 0; i < dataSize; ++i)
	{
		tempModel[i].pos.x = data[i].pos[0];
		tempModel[i].pos.y = data[i].pos[1];
		tempModel[i].pos.z = data[i].pos[2];
		tempModel[i].texCoords.x = data[i].uvw[0];
		tempModel[i].texCoords.y = data[i].uvw[1];
		tempModel[i].normal.x = data[i].nrm[0];
		tempModel[i].normal.y = data[i].nrm[1];
		tempModel[i].normal.z = data[i].nrm[2];
	}

	short* tempModelIndices = new short[indiciesSize];

	for (int i = 0; i < indiciesSize; ++i)
	{
		tempModelIndices[i] = indicies[i];
	}

	MakeVertexBuffer(&vBuffers[vBuffer], dataSize, tempModel);

	MakeIndexBuffer(&iBuffers[iBuffer], indiciesSize, tempModelIndices);

	delete[] tempModel;
	delete[] tempModelIndices;

	return true;
}

void DEMO_APP::MakeVertexBuffer(ID3D11Buffer** buffer, int size, SIMPLE_VERTEX* data)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * size;

	D3D11_SUBRESOURCE_DATA bufferResource;
	ZeroMemory(&bufferResource, sizeof(bufferResource));
	bufferResource.pSysMem = data;

	HRESULT result = device->CreateBuffer(&bufferDesc, &bufferResource, buffer);
}

void DEMO_APP::MakeVertexBuffer(ID3D11Buffer** buffer, int size, SKYBOX_VERTEX* data)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.ByteWidth = sizeof(SKYBOX_VERTEX) * size;

	D3D11_SUBRESOURCE_DATA bufferResource;
	ZeroMemory(&bufferResource, sizeof(bufferResource));
	bufferResource.pSysMem = data;

	HRESULT result = device->CreateBuffer(&bufferDesc, &bufferResource, buffer);
}

void DEMO_APP::MakeIndexBuffer(ID3D11Buffer** buffer, int size, short* data)
{
	D3D11_BUFFER_DESC indexDesc;
	ZeroMemory(&indexDesc, sizeof(indexDesc));
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = NULL;
	indexDesc.ByteWidth = sizeof(short) * size;

	D3D11_SUBRESOURCE_DATA indexResource;
	ZeroMemory(&indexResource, sizeof(indexResource));
	indexResource.pSysMem = data;

	HRESULT result = device->CreateBuffer(&indexDesc, &indexResource, buffer);
}

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

	RECT window_size = { 0, 0, backBuffer_width, backBuffer_height };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW /*& ~(WS_THICKFRAME | WS_MAXIMIZEBOX)*/,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	// TODO: PART 1 STEP 3a
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Width = backBuffer_width;
	swapChainDesc.BufferDesc.Height = backBuffer_height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;
	//swapChainDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	swapChainDesc.Windowed = true;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &device, NULL, &context);

	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();

	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = backBuffer_width;
	depthDesc.Height = backBuffer_height;
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

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthViewDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthBuffer, &depthViewDesc, &dsv);

	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;


	D3D11_TEXTURE2D_DESC rttDesc;
	ZeroMemory(&rttDesc, sizeof(rttDesc));
	rttDesc.Width = backBuffer_width;
	rttDesc.Height = backBuffer_height;
	rttDesc.MipLevels = 1;
	rttDesc.ArraySize = 1;
	rttDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rttDesc.SampleDesc.Count = 1;
	rttDesc.Usage = D3D11_USAGE_DEFAULT;
	rttDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rttDesc.CPUAccessFlags = 0;
	rttDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT result = device->CreateTexture2D(&rttDesc, NULL, &rtt_texture);

	D3D11_RENDER_TARGET_VIEW_DESC rtt_rtvDesc;
	ZeroMemory(&rtt_rtvDesc, sizeof(rtt_rtvDesc));
	rtt_rtvDesc.Format = rttDesc.Format;
	rtt_rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtt_rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(rtt_texture, &rtt_rtvDesc, &rtt_rtv);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtt_srvDesc;
	ZeroMemory(&rtt_srvDesc, sizeof(rtt_srvDesc));
	rtt_srvDesc.Format = rttDesc.Format;
	rtt_srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtt_srvDesc.Texture2D.MostDetailedMip = 0;
	rtt_srvDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(rtt_texture, &rtt_srvDesc, &rtt_shaderResource);

	D3D11_TEXTURE2D_DESC rtt_depthDesc;
	ZeroMemory(&rtt_depthDesc, sizeof(rtt_depthDesc));
	rtt_depthDesc.Width = backBuffer_width;
	rtt_depthDesc.Height = backBuffer_height;
	rtt_depthDesc.MipLevels = 1;
	rtt_depthDesc.ArraySize = 1;
	rtt_depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	rtt_depthDesc.SampleDesc.Count = 1;
	rtt_depthDesc.SampleDesc.Quality = 0;
	rtt_depthDesc.Usage = D3D11_USAGE_DEFAULT;
	rtt_depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	rtt_depthDesc.CPUAccessFlags = 0;
	rtt_depthDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&rtt_depthDesc, NULL, &rtt_depthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC rtt_depthViewDesc;
	ZeroMemory(&rtt_depthViewDesc, sizeof(rtt_depthViewDesc));
	rtt_depthViewDesc.Format = depthDesc.Format;
	rtt_depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	rtt_depthViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(rtt_depthBuffer, NULL, &rtt_dsv);

#if 1
	SIMPLE_VERTEX cubeVerts[] =
	{
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
	};

	short triangleVerts[] = {
		//Front Face
		16,19,17, 19,18,17,
		//Right Face
		13,15,12, 13,14,15,
		//Back Face
		21,23,20, 21,22,23,
		//Left Face
		8,11,9, 11,10,9,
		//Top Face
		0,3,1, 3,2,1,
		//Bottom Face
		5,7,4, 5,6,7
	};

	SKYBOX_VERTEX cubeMapVerts[] =
	{
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },

		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f) },
	};

	short cubeMapTriangleVerts[] = {
		//Front Face
		16,17,19, 19,17,18,
		//Right Face
		13,12,15, 13,15,14,
		//Back Face
		21,20,23, 21,23,22,
		//Left Face
		8,9,11, 11,9,10,
		//Top Face
		0,1,3, 3,1,2,
		//Bottom Face
		5,4,7, 5,7,6
	};
	
#endif

	MakeVertexBuffer(&vBuffers[1], ARRAYSIZE(cubeVerts), cubeVerts);
	MakeIndexBuffer(&iBuffers[1], ARRAYSIZE(triangleVerts), triangleVerts);

	result = CreateDDSTextureFromFile(device, L"greendragon.dds", nullptr, &diffuseViews[1]);
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"Barrel.dds", nullptr, &diffuseViews[2]);
	D3D11_SAMPLER_DESC barrelSampDesc;
	ZeroMemory(&barrelSampDesc, sizeof(barrelSampDesc));
	barrelSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	barrelSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	barrelSampDesc.MinLOD = 0;
	barrelSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"StoneHenge.dds", nullptr, &diffuseViews[3]);
	D3D11_SAMPLER_DESC SHSampDesc;
	ZeroMemory(&SHSampDesc, sizeof(SHSampDesc));
	SHSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SHSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SHSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SHSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SHSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SHSampDesc.MinLOD = 0;
	SHSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"earthmap1k.dds", nullptr, &diffuseViews[4]);
	D3D11_SAMPLER_DESC ESampDesc;
	ZeroMemory(&ESampDesc, sizeof(ESampDesc));
	ESampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ESampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ESampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ESampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ESampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ESampDesc.MinLOD = 0;
	ESampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"earthcloudmap.dds", nullptr, &diffuseViews[5]);
	D3D11_SAMPLER_DESC ECSampDesc;
	ZeroMemory(&ECSampDesc, sizeof(ECSampDesc));
	ECSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ECSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ECSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ECSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ECSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ECSampDesc.MinLOD = 0;
	ECSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"moonmap1k.dds", nullptr, &diffuseViews[6]);
	D3D11_SAMPLER_DESC MSampDesc;
	ZeroMemory(&MSampDesc, sizeof(MSampDesc));
	MSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	MSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	MSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	MSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	MSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	MSampDesc.MinLOD = 0;
	MSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = CreateDDSTextureFromFile(device, L"sunmap.dds", nullptr, &diffuseViews[7]);
	D3D11_SAMPLER_DESC SSampDesc;
	ZeroMemory(&SSampDesc, sizeof(SSampDesc));
	SSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SSampDesc.MinLOD = 0;
	SSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	MakeVertexBuffer(&vBuffers[0], ARRAYSIZE(cubeMapVerts), cubeMapVerts);
	MakeIndexBuffer(&iBuffers[0], ARRAYSIZE(cubeMapTriangleVerts), cubeMapTriangleVerts);

	result = CreateDDSTextureFromFile(device, L"OutputCube.dds", nullptr, &diffuseViews[0]);
	D3D11_SAMPLER_DESC cubeMapSampDesc;
	ZeroMemory(&cubeMapSampDesc, sizeof(cubeMapSampDesc));
	cubeMapSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	cubeMapSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	cubeMapSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	cubeMapSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	cubeMapSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	cubeMapSampDesc.MinLOD = 0;
	cubeMapSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = device->CreateSamplerState(&cubeMapSampDesc, &samplerStates[0]);

	result = device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	result = device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);
	result = device->CreateGeometryShader(Trivial_GS, sizeof(Trivial_GS), NULL, &gs);
	result = device->CreateVertexShader(SkyBox_VS, sizeof(SkyBox_VS), NULL, &skyBox_vs);
	result = device->CreatePixelShader(SkyBox_PS, sizeof(SkyBox_PS), NULL, &skyBox_ps);
	result = device->CreatePixelShader(RTT_PS, sizeof(RTT_PS), NULL, &rtt_ps);

	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SV_INSTANCEID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	D3D11_INPUT_ELEMENT_DESC skyBox_vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);
	result = device->CreateInputLayout(skyBox_vLayout, ARRAYSIZE(skyBox_vLayout), SkyBox_VS, sizeof(SkyBox_VS), &skyBox_inputLayout);

	cBufferVSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferVSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferVSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferVSDesc.ByteWidth = sizeof(SEND_TO_VS);

	result = device->CreateBuffer(&cBufferVSDesc, NULL, &cBufferVS);

	cBufferVSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferVSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferVSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferVSDesc.ByteWidth = sizeof(SEND_TO_VS);

	result = device->CreateBuffer(&cBufferVSDesc, NULL, &cBufferVS);

	cBufferPSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferPSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferPSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferPSDesc.ByteWidth = sizeof(SEND_TO_PS);

	result = device->CreateBuffer(&cBufferPSDesc, NULL, &cBufferPS);

	for (int i = 0; i < NUM_OF_MATRICIES; ++i)
	{
		worldMatricies[i] = DirectX::XMMatrixIdentity();
	}
	cameraMatrix = DirectX::XMMatrixRotationX(-18.0f);
	cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -5.0f), cameraMatrix);
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), backBuffer_width / backBuffer_height, 0.1f, 5000.0f);

	rtt_viewMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -5.0f), DirectX::XMMatrixRotationX(-18.0f)));
	rtt_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), 100 / 100, 0.1f, 100.0f);
	
	worldMatricies[3] = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);// DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f), worldMatricies[3]);
	worldMatricies[3].r[3] = cameraMatrix.r[3];

	LoadObjHeader(Barrel_data, ARRAYSIZE(Barrel_data), Barrel_indicies, ARRAYSIZE(Barrel_indicies), 2, 2);
	LoadObjHeader(StoneHenge_data, ARRAYSIZE(StoneHenge_data), StoneHenge_indicies, ARRAYSIZE(StoneHenge_indicies), 3, 3);
	LoadObjHeader(_3d_planet_data, ARRAYSIZE(_3d_planet_data), _3d_planet_indicies, ARRAYSIZE(_3d_planet_indicies), 4, 4);
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	time.Signal();

	CheckCameraInput();

	context->OMSetRenderTargets(1, &rtv, dsv);

	context->RSSetViewports(1, &viewport);

	FLOAT arr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	//
	// DRAW THE SKYBOX
	//
	toVSShader.worldMatrix = worldMatricies[3];
	toVSShader.viewMatrix = viewMatrix;
	toVSShader.projectionMatrix = projectionMatrix;

	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->VSSetConstantBuffers(2, 1, &cBufferVS);
	//context->PSSetConstantBuffers(1, 1, &cBufferPS);

	UINT stride = sizeof(SKYBOX_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vBuffers[0], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[0], DXGI_FORMAT_R16_UINT, offset);

	context->VSSetShader(skyBox_vs, 0, 0);
	context->PSSetShader(skyBox_ps, 0, 0);
	context->GSSetShader(nullptr, 0, 0);

	context->PSSetShaderResources(1, 1, &diffuseViews[0]);
	context->PSSetSamplers(1, 1, &samplerStates[0]);

	context->IASetInputLayout(skyBox_inputLayout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(36, 0, 0);

	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	//
	// DRAW THE FLAT CUBE RTT
	//
	worldMatricies[0] = DirectX::XMMatrixTranslation(0.0f, 0.25f, 0.0f);
	worldMatricies[0] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(10.0f, 1.0f, 10.0f), worldMatricies[0]);
	toVSShader.worldMatrix = worldMatricies[0];
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	toVSShader.viewMatrix = viewMatrix;
	toVSShader.projectionMatrix = projectionMatrix;

	toPSShader.lights[0].lightPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.lights[0].lightDirection = DirectX::XMFLOAT3(-0.3f, -sin(timer * 0.6f), -sin(timer * 0.6f));
	toPSShader.lights[0].lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.lights[1].lightPosition = DirectX::XMFLOAT4(0.0f, -2.0f, sin(timer), 1.0f);
	toPSShader.lights[1].lightDirection = DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f);
	toPSShader.lights[1].lightColor = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	toPSShader.lights[2].lightPosition = DirectX::XMFLOAT4(sin(timer * 1.5f), 2.0f, -sin(timer * 1.5f), 1.0f);//DirectX::XMFLOAT4(sin(timer * 2), 2.0f, sin(timer * 0.8f), 1.0f);
	toPSShader.lights[2].lightDirection = DirectX::XMFLOAT3(0.0f, -1.0f, 0.6f);
	toPSShader.lights[2].lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.ambientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//toPSShader.emissiveValue = DirectX::XMFLOAT4(1.0f, 1.0f, 0.2f, 0.0f);

	timer += time.SmoothDelta();

	//D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->VSSetConstantBuffers(0, 1, &cBufferVS);
	context->PSSetConstantBuffers(1, 1, &cBufferPS);

	stride = sizeof(SIMPLE_VERTEX);
	context->IASetVertexBuffers(0, 1, &vBuffers[1], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[1], DXGI_FORMAT_R16_UINT, offset);

	//context->OMSetRenderTargets(1, &rtv, dsv);

	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(rtt_ps, 0, 0);

	context->PSSetShaderResources(2, 1, &rtt_shaderResource);
	context->PSSetSamplers(0, 1, &samplerStates[0]);
	//context->PSSetSamplers(0, 1, &samplerStates[1]);

	context->IASetInputLayout(inputLayout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(36, 0, 0);

	//
	// DRAW THE FLAT CUBE
	//
	worldMatricies[8] = DirectX::XMMatrixTranslation(0.0f, -1.0f, 0.0f);
	worldMatricies[8] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(10.0f, 1.0f, 10.0f), worldMatricies[8]);
	toVSShader.worldMatrix = worldMatricies[8];

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->PSSetShaderResources(2, 1, &diffuseViews[1]);

	context->DrawIndexed(36, 0, 0);

	//
	// DRAW THE BARRELS
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[2], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[2], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShader(ps, 0, 0);
	//context->GSSetShader(gs, 0, 0);

	context->PSSetShaderResources(0, 1, &diffuseViews[2]);
	//context->PSSetSamplers(0, 1, &samplerStates[2]);

	worldMatricies[2] = DirectX::XMMatrixTranslation(0.0f, -2.5f, 2.0f);
	worldMatricies[2] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.05f, 0.05f, 0.05f), worldMatricies[2]);
	toVSShader.worldMatrix = worldMatricies[2];

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	//context->GSSetConstantBuffers(3, 1, &cBufferVS);

	context->DrawIndexedInstanced(ARRAYSIZE(Barrel_indicies), 10, 0, 0, 0);

	//
	// DRAW THE SUN
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[4], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[4], DXGI_FORMAT_R16_UINT, offset);

	context->GSSetShader(nullptr, 0, 0);

	context->PSSetShaderResources(0, 1, &diffuseViews[7]);
	//context->PSSetSamplers(0, 1, &samplerStates[4]);

	worldMatricies[5] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 400.0f);
	worldMatricies[5] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[5]);
	worldMatricies[5] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f), worldMatricies[5]);
	toVSShader.worldMatrix = worldMatricies[5];
	//toPSShader.emissiveValue = DirectX::XMFLOAT4(1.0f, 1.0f, 0.2f, 0.7f);

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	//context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	//context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(ARRAYSIZE(_3d_planet_indicies), 0, 0);

	//
	// DRAW THE EARTH
	//
	context->PSSetShaderResources(0, 1, &diffuseViews[4]);
	context->PSSetShaderResources(3, 1, &diffuseViews[5]);

	worldMatricies[6] = DirectX::XMMatrixTranslation(worldMatricies[5].r[3].m128_f32[0] + 0.0f, worldMatricies[5].r[3].m128_f32[1] + 0.0f, worldMatricies[5].r[3].m128_f32[2] + 0.0f);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(250.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(timer * 0.15f)), worldMatricies[6]);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[6]);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f), worldMatricies[6]);
	toVSShader.worldMatrix = worldMatricies[6];
	//toPSShader.emissiveValue = DirectX::XMFLOAT4(1.0f, 1.0f, 0.2f, 0.0f);

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	//context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	//context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(ARRAYSIZE(_3d_planet_indicies), 0, 0);

	//
	// DRAW THE MOON
	//
	context->PSSetShaderResources(0, 1, &diffuseViews[6]);
	context->PSSetShaderResources(3, 1, pSRV);

	worldMatricies[7] = DirectX::XMMatrixTranslation(worldMatricies[6].r[3].m128_f32[0] + 0.0f, worldMatricies[6].r[3].m128_f32[1] + 0.0f, worldMatricies[6].r[3].m128_f32[2] + 0.0f);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-100.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(timer * 0.7f)), worldMatricies[7]);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.65f), worldMatricies[7]);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.05f, 0.05f, 0.05f), worldMatricies[7]);
	toVSShader.worldMatrix = worldMatricies[7];

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->DrawIndexed(ARRAYSIZE(_3d_planet_indicies), 0, 0);

	//
	//DRAW CUBES ONTO A TEXTURE TO BE RENDERED
	//
	context->OMSetRenderTargets(1, &rtt_rtv, rtt_dsv);
	arr[0] = 1.0f;
	arr[1] = 1.0f;
	arr[2] = 1.0f;
	context->ClearRenderTargetView(rtt_rtv, arr);
	context->ClearDepthStencilView(rtt_dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	//
	// DRAW STONEHENGE ONTO A TEXTURE TO BE RENDERED
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[3], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[3], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShaderResources(0, 1, &diffuseViews[3]);
	context->PSSetShaderResources(3, 1, pSRV);
	//context->PSSetSamplers(0, 1, &samplerStates[3]);

	worldMatricies[4] = DirectX::XMMatrixTranslation(0.0f, -15.0f, 15.0f);
	worldMatricies[4] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[4]);
	toVSShader.worldMatrix = worldMatricies[4];
	toVSShader.viewMatrix = rtt_viewMatrix;
	toVSShader.projectionMatrix = rtt_projectionMatrix;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->DrawIndexed(ARRAYSIZE(StoneHenge_indicies), 0, 0);

	context->GenerateMips(rtt_shaderResource);

	swapChain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	backBuffer->Release();
	device->Release();
	swapChain->Release();
	depthBuffer->Release();
	rtv->Release();
	dsv->Release();
	context->Release();
	cBufferVS->Release();
	cBufferPS->Release();
	ps->Release();
	vs->Release();
	gs->Release();
	skyBox_ps->Release();
	skyBox_vs->Release();
	inputLayout->Release();
	skyBox_inputLayout->Release();
	rtt_depthBuffer->Release();
	rtt_dsv->Release();
	rtt_ps->Release();
	rtt_rtv->Release();
	rtt_shaderResource->Release();
	rtt_texture->Release();

	for (unsigned int i = 0; i < NUM_OF_VBUFFERS; ++i)
	{
		vBuffers[i]->Release();
	}
	for (unsigned int i = 0; i < NUM_OF_IBUFFERS; ++i)
	{
		iBuffers[i]->Release();
	}
	for (unsigned int i = 0; i < NUM_OF_DVIEWS; ++i)
	{
		diffuseViews[i]->Release();
	}
	for (unsigned int i = 0; i < NUM_OF_SSTATES; ++i)
	{
		samplerStates[i]->Release();
	}

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	myApp = new DEMO_APP(hInstance, (WNDPROC)DEMO_APP::WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp->Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp->ShutDown();
	return 0;
}
LRESULT CALLBACK DEMO_APP::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY): { PostQuitMessage(0); }
					   break;
	case (WM_SIZE):
	{
		if (myApp)
		{
			myApp->context->OMSetRenderTargets(0, 0, 0);

			myApp->rtv->Release();

			HRESULT hr;
			hr = myApp->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

			ID3D11Texture2D* pBuffer;
			hr = myApp->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBuffer);

			myApp->device->CreateRenderTargetView(pBuffer, NULL, &myApp->rtv);

			pBuffer->Release();

			DXGI_SWAP_CHAIN_DESC tempSwapDesc;
			ZeroMemory(&tempSwapDesc, sizeof(tempSwapDesc));
			myApp->swapChain->GetDesc(&tempSwapDesc);

			myApp->backBuffer_width = tempSwapDesc.BufferDesc.Width;
			myApp->backBuffer_height = tempSwapDesc.BufferDesc.Height;

			myApp->viewport.Width = myApp->backBuffer_width;
			myApp->viewport.Height = myApp->backBuffer_height;
			myApp->viewport.MinDepth = 0.0f;
			myApp->viewport.MaxDepth = 1.0f;
			myApp->viewport.TopLeftX = 0;
			myApp->viewport.TopLeftY = 0;

			myApp->context->RSSetViewports(1, &myApp->viewport);

			myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), myApp->backBuffer_width / myApp->backBuffer_height, 0.1f, 5000.0f);

			D3D11_TEXTURE2D_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.Width = myApp->backBuffer_width;
			depthDesc.Height = myApp->backBuffer_height;
			depthDesc.MipLevels = 1;
			depthDesc.ArraySize = 1;
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthDesc.SampleDesc.Count = 1;
			depthDesc.SampleDesc.Quality = 0;
			depthDesc.Usage = D3D11_USAGE_DEFAULT;
			depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthDesc.CPUAccessFlags = 0;
			depthDesc.MiscFlags = 0;

			myApp->device->CreateTexture2D(&depthDesc, NULL, &myApp->depthBuffer);

			myApp->device->CreateDepthStencilView(myApp->depthBuffer, NULL, &myApp->dsv);

			myApp->context->OMSetRenderTargets(1, &myApp->rtv, myApp->dsv);
		}
	}
					break;
	case (WM_LBUTTONDOWN):
	{
		RECT rect;
		POINT pt;
		GetWindowRect(hWnd, &rect);
		GetCursorPos(&pt);

		myApp->tempPointDown = pt;
		
		if (DragDetect(hWnd, pt))
		{
			SetTimer(hWnd, TID_POLLMOUSE, MOUSE_POLL_DELAY, NULL);
		}
	}
					break;
	case (WM_TIMER):
	{
		RECT rect;
		POINT pt;
		GetWindowRect(hWnd, &rect);
		GetPhysicalCursorPos(&pt);

		int num = GetSystemMetrics(SM_CXDRAG);
		if (myApp->tempPointDown.x != NULL)
		{
			int x = myApp->tempPointDown.y - pt.y;
			int y = myApp->tempPointDown.x - pt.x;

			DirectX::XMVECTOR tempVector = myApp->cameraMatrix.r[3];

			myApp->cameraMatrix.r[3] = DirectX::XMLoadFloat4(&DirectX::XMFLOAT4(0, 0, 0, 1));

			myApp->cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(x * -0.005f), myApp->cameraMatrix);
			myApp->cameraMatrix = DirectX::XMMatrixMultiply(myApp->cameraMatrix, DirectX::XMMatrixRotationY(y * -0.005f));


			myApp->cameraMatrix.r[3] = tempVector;
		}
		myApp->tempPointDown = pt;
	}
					break;
	case (WM_LBUTTONUP):
	{
		KillTimer(hWnd, TID_POLLMOUSE);
	}
					break;
	case (WM_MOUSEWHEEL):
	{
		myApp->fov += -(GET_WHEEL_DELTA_WPARAM(wParam) / 120);

		myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), myApp->backBuffer_width / myApp->backBuffer_height, 0.1f, 5000.0f);
	}
					break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//
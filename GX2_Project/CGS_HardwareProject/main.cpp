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

#define TID_POLLMOUSE 100
#define MOUSE_POLL_DELAY 5

using namespace std;

#include <d3d11.h>
#pragma comment(lib, "d3d11");

#include <DirectXMath.h>

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"

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
	ID3D11Texture2D* backBuffer;
	ID3D11Texture2D* depthBuffer;

	float backBuffer_width = 500;
	float backBuffer_height = 500;

	ID3D11Buffer* buffer;

	ID3D11InputLayout* inputLayout;

	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* barrelIndexBuffer;

	ID3D11Resource* diffuseResource;
	ID3D11ShaderResourceView* diffuseView;
	ID3D11SamplerState* samplerState;

	ID3D11Buffer* barrelBuffer;
	ID3D11ShaderResourceView* barrelDiffuseView;
	ID3D11SamplerState* barrelSamplerState;

		// TODO: PART 2 STEP 4
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	// BEGIN PART 3
	// TODO: PART 3 STEP 1
	ID3D11Buffer* cBufferVS;
	D3D11_BUFFER_DESC cBufferVSDesc = {};
	ID3D11Buffer* cBufferPS;
	D3D11_BUFFER_DESC cBufferPSDesc = {};
	XTime time;
	float timer = 0.0;
	float cameraTimer = 0.0;

	DirectX::XMMATRIX worldMatrix1; //big cube
	DirectX::XMMATRIX worldMatrix2; //smaller cube
	DirectX::XMMATRIX worldMatrix3; //barrel
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

	struct SEND_TO_PS
	{
		LIGHTS lights[3];
		DirectX::XMFLOAT4 ambientColor;
	};

	// TODO: PART 3 STEP 4a
	SEND_TO_VS toVSShader;
	SEND_TO_PS toPSShader;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoords;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run(); 
	bool ShutDown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

	bool LoadObjHeader();
};

DEMO_APP* myApp = nullptr;

bool DEMO_APP::LoadObjHeader()
{
	SIMPLE_VERTEX tempModel[ARRAYSIZE(Barrel_data)];

	for (int i = 0; i < ARRAYSIZE(Barrel_data); ++i)
	{
		tempModel[i].pos.x = Barrel_data[i].pos[0];
		tempModel[i].pos.y = Barrel_data[i].pos[1];
		tempModel[i].pos.z = Barrel_data[i].pos[2];
		tempModel[i].texCoords.x = Barrel_data[i].uvw[0];
		tempModel[i].texCoords.y = Barrel_data[i].uvw[1];
		tempModel[i].normal.x = Barrel_data[i].nrm[0];
		tempModel[i].normal.y = Barrel_data[i].nrm[1];
		tempModel[i].normal.z = Barrel_data[i].nrm[2];
	}

	short tempModelIndices[ARRAYSIZE(Barrel_indicies)];

	for (int i = 0; i < ARRAYSIZE(Barrel_indicies); ++i)
	{
		tempModelIndices[i] = Barrel_indicies[i];
	}

	D3D11_BUFFER_DESC barrelBufferDesc = {};
	barrelBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	barrelBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	barrelBufferDesc.CPUAccessFlags = NULL;
	barrelBufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * ARRAYSIZE(tempModel);

	D3D11_SUBRESOURCE_DATA barrelBufferResource = {};
	barrelBufferResource.pSysMem = tempModel;

	device->CreateBuffer(&barrelBufferDesc, &barrelBufferResource, &barrelBuffer);

	D3D11_BUFFER_DESC barrelIndexDesc = {};
	barrelIndexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	barrelIndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	barrelIndexDesc.CPUAccessFlags = NULL;
	barrelIndexDesc.ByteWidth = sizeof(short) * ARRAYSIZE(tempModelIndices);

	barrelBufferResource.pSysMem = tempModelIndices;

	device->CreateBuffer(&barrelIndexDesc, &barrelBufferResource, &barrelIndexBuffer);

	return true;
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

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(/*WS_THICKFRAME | */WS_MAXIMIZEBOX),
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
	//swapChainDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	swapChainDesc.Windowed = true;

	// TODO: PART 1 STEP 3b
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &device, NULL, &context);

	// TODO: PART 1 STEP 4
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();

	D3D11_TEXTURE2D_DESC depthDesc;
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
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthViewDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthBuffer, NULL, &dsv);

	// TODO: PART 1 STEP 5
	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// TODO: PART 2 STEP 3a
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
	
#endif

	// BEGIN PART 4
	// TODO: PART 4 STEP 1

	// TODO: PART 2 STEP 3b
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * ARRAYSIZE(cubeVerts);

	// TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA bufferResource = {};
	bufferResource.pSysMem = cubeVerts;

	device->CreateBuffer(&bufferDesc, &bufferResource, &buffer);

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = NULL;
	indexDesc.ByteWidth = sizeof(short) * ARRAYSIZE(triangleVerts);

	// TODO: PART 2 STEP 3d
	bufferResource.pSysMem = triangleVerts;

	device->CreateBuffer(&indexDesc, &bufferResource, &indexBuffer);

	HRESULT result = CreateDDSTextureFromFile(device, L"greendragon.dds", nullptr, &diffuseView);
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = device->CreateSamplerState(&sampDesc, &samplerState);

	result = CreateDDSTextureFromFile(device, L"Barrel.dds", nullptr, &barrelDiffuseView);

	D3D11_SAMPLER_DESC barrelSampDesc;
	ZeroMemory(&barrelSampDesc, sizeof(barrelSampDesc));
	barrelSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	barrelSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	barrelSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	barrelSampDesc.MinLOD = 0;
	barrelSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = device->CreateSamplerState(&barrelSampDesc, &barrelSamplerState);
	
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	// TODO: PART 2 STEP 8a
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// TODO: PART 2 STEP 8b
	device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	// TODO: PART 3 STEP 3
	cBufferVSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferVSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferVSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferVSDesc.ByteWidth = sizeof(SEND_TO_VS);

	device->CreateBuffer(&cBufferVSDesc, NULL, &cBufferVS);

	cBufferPSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferPSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferPSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferPSDesc.ByteWidth = sizeof(SEND_TO_VS);

	device->CreateBuffer(&cBufferPSDesc, NULL, &cBufferPS);

	// TODO: PART 3 STEP 4b
	worldMatrix1 = DirectX::XMMatrixIdentity();
	worldMatrix2 = DirectX::XMMatrixIdentity();
	worldMatrix3 = DirectX::XMMatrixIdentity();
	cameraMatrix = DirectX::XMMatrixRotationX(-18.0f);
	cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -5.0f), cameraMatrix);
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), backBuffer_width / backBuffer_height, 0.1f, 100.0f);
	
	LoadObjHeader();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	time.Signal();

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
		if (GetAsyncKeyState(0x51)) // 'Q'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(cameraMatrix, tempM);

			cameraTimer = 0.0f;
		}
		if (GetAsyncKeyState(0x45)) // 'E'
		{
			DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, -0.05f, 0.0f);

			cameraMatrix = DirectX::XMMatrixMultiply(cameraMatrix, tempM);

			cameraTimer = 0.0f;
		}
	}
	else
	{
		cameraTimer += time.SmoothDelta();
	}

	context->OMSetRenderTargets(1, &rtv, dsv);

	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);

	// TODO: PART 1 STEP 7c
	FLOAT arr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	worldMatrix1 = DirectX::XMMatrixTranslation(0.0f, 0.25f, 0.0f);//DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(-timer), DirectX::XMMatrixTranslation(0.0f, 0.25f, 0.0f));
	worldMatrix1 = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(10.0f, 1.0f, 10.0f), worldMatrix1);
	toVSShader.worldMatrix = worldMatrix1;
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	toVSShader.viewMatrix = viewMatrix;
	toVSShader.projectionMatrix = projectionMatrix;

	toPSShader.lights[0].lightPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.lights[0].lightDirection = DirectX::XMFLOAT3(-0.3f, -sin(timer * 0.6f), -0.3f);
	toPSShader.lights[0].lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.lights[1].lightPosition = DirectX::XMFLOAT4(0.0f, sin(timer * 1.2f) + 1.5f, 0.0f, 1.0f);
	toPSShader.lights[1].lightDirection = DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f);
	toPSShader.lights[1].lightColor = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	toPSShader.lights[2].lightPosition = DirectX::XMFLOAT4(sin(timer * 2), 2.0f, sin(timer * 0.8f), 1.0f);
	toPSShader.lights[2].lightDirection = DirectX::XMFLOAT3(0.3f, -1.0f, 0.3f);
	toPSShader.lights[2].lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.ambientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	timer += time.SmoothDelta();

	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->VSSetConstantBuffers(0, 1, &cBufferVS);
	context->PSSetConstantBuffers(1, 1, &cBufferPS);

	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, offset);

	// TODO: PART 2 STEP 9b
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	context->PSSetShaderResources(0, 1, &diffuseView);
	context->PSSetSamplers(0, 1, &samplerState);

	// TODO: PART 2 STEP 9c
	context->IASetInputLayout(inputLayout);

	// TODO: PART 2 STEP 9d
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// TODO: PART 2 STEP 10
	context->DrawIndexed(36, 0, 0);

	worldMatrix2 = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(2.0f, 0.25f, 0.0f), DirectX::XMMatrixRotationZ(timer * 0.8f));
	worldMatrix2 = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(timer * 0.2f), DirectX::XMMatrixRotationY(timer * 1.5f)), DirectX::XMMatrixRotationZ(timer * 0.8f)), worldMatrix2);
	worldMatrix2 = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f), worldMatrix2);
	toVSShader.worldMatrix = worldMatrix2;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->DrawIndexed(36, 0, 0);

	context->IASetVertexBuffers(0, 1, &barrelBuffer, &stride, &offset);
	context->IASetIndexBuffer(barrelIndexBuffer, DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShaderResources(0, 1, &barrelDiffuseView);
	context->PSSetSamplers(0, 1, &barrelSamplerState);

	worldMatrix3 = DirectX::XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	worldMatrix3 = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.05f, 0.05f, 0.05f), worldMatrix3);
	toVSShader.worldMatrix = worldMatrix3;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->DrawIndexed(ARRAYSIZE(Barrel_indicies), 0, 0);

	swapChain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	device->Release();
	swapChain->Release();
	rtv->Release();
	context->Release();
	cBufferVS->Release();
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
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
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

			myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), myApp->backBuffer_width / myApp->backBuffer_height, 0.1f, 100.0f);

			D3D11_TEXTURE2D_DESC depthDesc;
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

			DirectX::XMMATRIX tempMX = DirectX::XMMatrixRotationX(x * -0.005f);
			DirectX::XMMATRIX tempMY = DirectX::XMMatrixRotationY(y * -0.005f);

			myApp->cameraMatrix = DirectX::XMMatrixMultiply(tempMX, myApp->cameraMatrix);
			myApp->cameraMatrix = DirectX::XMMatrixMultiply(myApp->cameraMatrix, tempMY);

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
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//
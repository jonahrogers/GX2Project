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
#include <vector>
//#include <fstream>
#include "DDSTextureLoader.h"

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

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

float backBuffer_width = 500;
float backBuffer_height = 500;

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

	// TODO: PART 2 STEP 2
	ID3D11Buffer* buffer;
	unsigned int vertCount;
	ID3D11InputLayout* inputLayout;

	ID3D11Buffer* indexBuffer;

	ID3D11Texture2D* diffuseTexture;
	ID3D11Resource* diffuseResource;
	ID3D11ShaderResourceView* diffuseView;
	ID3D11SamplerState* samplerState;

		// BEGIN PART 5
		// TODO: PART 5 STEP 1

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
	POINT tempPointDown = { MININT, MININT };
	DirectX::XMMATRIX projectionMatrix;

	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		
		//DirectX::XMFLOAT3 lightDirection;
		//DirectX::XMFLOAT4 ambientColor;
	};

	//struct SEND_TO_PS
	//{
	//	DirectX::XMFLOAT4 vMeshColor;
	//};

	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;
	//SEND_TO_PS toPSShader;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoords;
		//DirectX::XMFLOAT3 normal;
		//DirectX::XMFLOAT4 color;
	};

	SIMPLE_VERTEX* barrel;

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run(); 
	bool ShutDown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
};

DEMO_APP* myApp = nullptr;

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

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Width = backBuffer_width;
	swapChainDesc.BufferDesc.Height = backBuffer_height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &device, NULL, &context);

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

	device->CreateDepthStencilView(depthBuffer, NULL, &dsv);

	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	SIMPLE_VERTEX cubeVerts[] =
	{
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f)},
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f)},

		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 1.0f),},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 1.0f),},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(1.0f, 0.0f),},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f), DirectX::XMFLOAT2(0.0f, 0.0f),},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(0.0f, 0.0f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f), DirectX::XMFLOAT2(1.0f, 0.0f)},
	};

	short triangleVerts[] = 
	{
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

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * ARRAYSIZE(cubeVerts);

	D3D11_SUBRESOURCE_DATA bufferResource = {};
	bufferResource.pSysMem = cubeVerts;

	device->CreateBuffer(&bufferDesc, &bufferResource, &buffer);

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = NULL;
	indexDesc.ByteWidth = sizeof(short) * ARRAYSIZE(triangleVerts);

	bufferResource.pSysMem = triangleVerts;

	device->CreateBuffer(&indexDesc, &bufferResource, &indexBuffer);

	HRESULT result = DirectX::CreateDDSTextureFromFile(device, L"greendragon.dds", nullptr, &diffuseView, 0, nullptr);

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

	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.ByteWidth = sizeof(SEND_TO_VRAM);

	device->CreateBuffer(&cBufferDesc, NULL, &cBuffer);

	worldMatrix1 = DirectX::XMMatrixIdentity();
	worldMatrix2 = DirectX::XMMatrixIdentity();
	//cameraMatrix = DirectX::XMMatrixLookToLH();
	cameraMatrix = DirectX::XMMatrixRotationX(-18.0f);
	cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -1.0f), cameraMatrix);
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), backBuffer_width / backBuffer_height, 0.1f, 100.0f);


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
	// END PART 4

	// TODO: PART 1 STEP 7a
	context->OMSetRenderTargets(1, &rtv, dsv);

	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);

	// TODO: PART 1 STEP 7c
	FLOAT arr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	// TODO: PART 5 STEP 4

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

	//toShader.lightDirection = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	//toShader.ambientColor = DirectX::XMFLOAT4(0.1f, 0.3f, 0.3f, 1.0f);

	timer += time.SmoothDelta();

	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toShader, sizeof(toShader));
	context->Unmap(cBuffer, NULL);

	// TODO: PART 3 STEP 6
	context->VSSetConstantBuffers(0, 1, &cBuffer);
	context->PSSetConstantBuffers(0, 1, &cBuffer);

	// TODO: PART 2 STEP 9a
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

			backBuffer_width = tempSwapDesc.BufferDesc.Width;
			backBuffer_height = tempSwapDesc.BufferDesc.Height;

			//D3D11_VIEWPORT vp;
			myApp->viewport.Width = backBuffer_width;
			myApp->viewport.Height = backBuffer_height;
			myApp->viewport.MinDepth = 0.0f;
			myApp->viewport.MaxDepth = 1.0f;
			myApp->viewport.TopLeftX = 0;
			myApp->viewport.TopLeftY = 0;

			myApp->context->RSSetViewports(1, &myApp->viewport);

			myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), backBuffer_width / backBuffer_height, 0.1f, 100.0f);

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

			myApp->device->CreateTexture2D(&depthDesc, NULL, &myApp->depthBuffer);

			myApp->device->CreateDepthStencilView(myApp->depthBuffer, NULL, &myApp->dsv);

			myApp->context->OMSetRenderTargets(1, &myApp->rtv, myApp->dsv);
		}
	}
					break;
	case (WM_LBUTTONDOWN):
	{
#define TID_POLLMOUSE 100
#define MOUSE_POLL_DELAY 5

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

			DirectX::XMMATRIX tempMX = DirectX::XMMatrixRotationX(x * 0.002f);
			DirectX::XMMATRIX tempMY = DirectX::XMMatrixRotationY(y * 0.002f);

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
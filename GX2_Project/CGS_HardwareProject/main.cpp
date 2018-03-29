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

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>

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
	D3D11_VIEWPORT viewport;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IDXGISwapChain* swapChain;

	// TODO: PART 2 STEP 2
	ID3D11Buffer* buffer;
	unsigned int vertCount;
	ID3D11InputLayout* inputLayout;

	D3D11_INPUT_ELEMENT_DESC vLayout;

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

	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		DirectX::XMFLOAT4 constantColor;
		DirectX::XMFLOAT2 constantOffset;
		DirectX::XMFLOAT2 padding;
	};

	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;

	bool right = true;
	bool left = false;
	bool top = true;
	bool bottom = false;

	SEND_TO_VRAM toGridShader;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT3 pos;
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

	// TODO: PART 1 STEP 5
	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// TODO: PART 2 STEP 3a
	//SIMPLE_VERTEX circleVerts[361];
	SIMPLE_VERTEX circleVerts[8];

	SIMPLE_VERTEX triangleVerts[36];

	/*for (unsigned int i = 0; i <= 360; ++i)
	{
		if (i == 360)
		{
			circleVerts[i] = circleVerts[0];
		}
		else
		{
		circleVerts[i].pos.x = sin(DirectX::XM_PI / 180.0f * i);
		circleVerts[i].pos.y = cos(DirectX::XM_PI / 180.0f * i);
		}
	}*/

#if 1
	//Bottom Left, -z
	circleVerts[0].pos.x = -0.25f;
	circleVerts[0].pos.y = -0.25f;
	circleVerts[0].pos.z = -0.25f;
	//Bottom Left, +z
	circleVerts[1].pos.x = -0.25f;
	circleVerts[1].pos.y = -0.25f;
	circleVerts[1].pos.z = 0.25f;
	//Bottom Right, +z
	circleVerts[2].pos.x = 0.25f;
	circleVerts[2].pos.y = -0.25f;
	circleVerts[2].pos.z = 0.25f;
	//Bottom Right, -z
	circleVerts[3].pos.x = 0.25f;
	circleVerts[3].pos.y = -0.25f;
	circleVerts[3].pos.z = -0.25f;
	//Top Left, -z
	circleVerts[4].pos.x = -0.25f;
	circleVerts[4].pos.y = 0.25f;
	circleVerts[4].pos.z = -0.25f;
	//Top Left, +z
	circleVerts[5].pos.x = -0.25f;
	circleVerts[5].pos.y = 0.25f;
	circleVerts[5].pos.z = 0.25f;
	//Top Right, +z
	circleVerts[6].pos.x = 0.25f;
	circleVerts[6].pos.y = 0.25f;
	circleVerts[6].pos.z = 0.25f;
	//Top Right, -z
	circleVerts[7].pos.x = 0.25f;
	circleVerts[7].pos.y = 0.25f;
	circleVerts[7].pos.z = -0.25f;

	//Front Face
	triangleVerts[0] = circleVerts[0];
	triangleVerts[1] = circleVerts[4];
	triangleVerts[2] = circleVerts[3];
	triangleVerts[3] = circleVerts[4];
	triangleVerts[4] = circleVerts[7];
	triangleVerts[5] = circleVerts[3];
	//Right Face
	triangleVerts[6] = circleVerts[3];
	triangleVerts[7] = circleVerts[7];
	triangleVerts[8] = circleVerts[2];
	triangleVerts[9] = circleVerts[7];
	triangleVerts[10] = circleVerts[6];
	triangleVerts[11] = circleVerts[2];
	//Back Face
	triangleVerts[12] = circleVerts[2];
	triangleVerts[13] = circleVerts[6];
	triangleVerts[14] = circleVerts[1];
	triangleVerts[15] = circleVerts[6];
	triangleVerts[16] = circleVerts[5];
	triangleVerts[17] = circleVerts[1];
	//Left Face
	triangleVerts[18] = circleVerts[1];
	triangleVerts[19] = circleVerts[5];
	triangleVerts[20] = circleVerts[0];
	triangleVerts[21] = circleVerts[5];
	triangleVerts[22] = circleVerts[4];
	triangleVerts[23] = circleVerts[0];
	//Top Face
	triangleVerts[24] = circleVerts[4];
	triangleVerts[25] = circleVerts[5];
	triangleVerts[26] = circleVerts[7];
	triangleVerts[27] = circleVerts[5];
	triangleVerts[28] = circleVerts[6];
	triangleVerts[29] = circleVerts[7];
	//Bottom Face
	triangleVerts[30] = circleVerts[0];
	triangleVerts[31] = circleVerts[1];
	triangleVerts[32] = circleVerts[3];
	triangleVerts[33] = circleVerts[1];
	triangleVerts[34] = circleVerts[2];
	triangleVerts[35] = circleVerts[3];
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
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 8;

	// TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA bufferResource;

	bufferResource.pSysMem = triangleVerts;

	// TODO: PART 2 STEP 3d
	device->CreateBuffer(&bufferDesc, &bufferResource, &buffer);

	// TODO: PART 5 STEP 2a
	SIMPLE_VERTEX gridVerts[1200];

	// TODO: PART 5 STEP 2b
	DirectX::XMFLOAT2 vert1 = { -0.9f, 1.0f };
	DirectX::XMFLOAT2 vert2 = { 0, 0 };
	DirectX::XMFLOAT2 vert3 = { 0, 0 };
	DirectX::XMFLOAT2 vert4 = { 0, 0 };
	DirectX::XMFLOAT2 vert5 = { 0, 0 };
	DirectX::XMFLOAT2 vert6 = { 0, 0 };
	bool isEven = false;

	for (unsigned int i = 0; i < 20; i++)
	{
		for (unsigned int j = 0; j < 10; ++j)
		{
			if (isEven == false && vert1.x > 0.9)
			{
				vert1.x = -1.0;
				vert1.y -= 0.1;

				isEven = true;
			}
			else if (isEven == true && vert1.x > 0.8)
			{
				vert1.x = -0.9;
				vert1.y -= 0.1;

				isEven = false;
			}


			vert2.x = vert1.x + 0.1;
			vert2.y = vert1.y;

			vert3.x = vert1.x;
			vert3.y = vert1.y - 0.1;

			vert4 = vert2;

			vert5.x = vert2.x;
			vert5.y = vert3.y;

			vert6 = vert3;

			gridVerts[gridVertCount].pos.x = vert1.x;
			gridVerts[gridVertCount].pos.y = vert1.y;
			gridVertCount++;

			gridVerts[gridVertCount].pos.x = vert2.x;
			gridVerts[gridVertCount].pos.y = vert2.y;
			gridVertCount++;

			gridVerts[gridVertCount].pos.x = vert3.x;
			gridVerts[gridVertCount].pos.y = vert3.y;
			gridVertCount++;

			gridVerts[gridVertCount].pos.x = vert4.x;
			gridVerts[gridVertCount].pos.y = vert4.y;
			gridVertCount++;

			gridVerts[gridVertCount].pos.x = vert5.x;
			gridVerts[gridVertCount].pos.y = vert5.y;
			gridVertCount++;

			gridVerts[gridVertCount].pos.x = vert6.x;
			gridVerts[gridVertCount].pos.y = vert6.y;
			gridVertCount++;

			vert1.x += 0.2;
		}
	}
	
	// TODO: PART 5 STEP 3
	D3D11_BUFFER_DESC gridBufferDesc = {};

	gridBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	gridBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	gridBufferDesc.CPUAccessFlags = NULL;
	gridBufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 1200;
	gridBufferDesc.StructureByteStride = sizeof(SIMPLE_VERTEX);
	gridBufferDesc.MiscFlags = NULL;

	D3D11_SUBRESOURCE_DATA gridBufferResource;

	gridBufferResource.pSysMem = gridVerts;
	gridBufferResource.SysMemPitch = 0;
	gridBufferResource.SysMemSlicePitch = 0;

	device->CreateBuffer(&gridBufferDesc, &gridBufferResource, &gridBuffer);

	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	// TODO: PART 2 STEP 8a
	vLayout =
	{
		"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
	};

	// TODO: PART 2 STEP 8b
	device->CreateInputLayout(&vLayout, 1, Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	// TODO: PART 3 STEP 3
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.ByteWidth = sizeof(SEND_TO_VRAM);

	device->CreateBuffer(&cBufferDesc, NULL, &cBuffer);

	// TODO: PART 3 STEP 4b
	toShader.constantOffset = { 0, 0 };
	toShader.constantColor = { 1.0f, 1.0f, 0.0f, 1.0f };

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	time.Signal();

	// TODO: PART 4 STEP 3
	SIMPLE_VERTEX velocityVector;
	velocityVector.pos.x = 1.0f;
	velocityVector.pos.y = 0.5f;
	
	// TODO: PART 4 STEP 5

	if (toShader.constantOffset.x >= 1.0f)
	{
		right = false;
		left = true;
	}
	if (toShader.constantOffset.x <= -1.0f)
	{
		left = false;
		right = true;
	}

	if (toShader.constantOffset.y >= 1.0f)
	{
		top = false;
		bottom = true;
	}
	if (toShader.constantOffset.y <= -1.0f)
	{
		bottom = false;
		top = true;
	}

	if (right)
		toShader.constantOffset.x += velocityVector.pos.x * float(time.Delta());
	else if (left)
		toShader.constantOffset.x -= velocityVector.pos.x * float(time.Delta());

	if (top)
		toShader.constantOffset.y += velocityVector.pos.y * float(time.Delta());
	else if (bottom)
		toShader.constantOffset.y -= velocityVector.pos.y * float(time.Delta());

	// END PART 4

	// TODO: PART 1 STEP 7a
	context->OMSetRenderTargets(1, &rtv, NULL);

	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);

	// TODO: PART 1 STEP 7c
	FLOAT arr[4] = { 0.0f, 0.0f, 0.7f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);

	// TODO: PART 5 STEP 4
	toGridShader.constantColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	toGridShader.constantOffset = { 0,0 };
	toGridShader.padding = { 0.0f, 0.0f };

	// TODO: PART 5 STEP 5
	D3D11_MAPPED_SUBRESOURCE resource2;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &resource2);
	memcpy(resource2.pData, &toGridShader, sizeof(toGridShader));
	context->Unmap(cBuffer, NULL);

	// TODO: PART 5 STEP 6
	context->VSSetConstantBuffers(0, 1, &cBuffer);

	UINT gridStride = sizeof(SIMPLE_VERTEX);
	UINT gridOffset = 0;
	context->IASetVertexBuffers(0, 1, &gridBuffer, &gridStride, &gridOffset);
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// TODO: PART 5 STEP 7
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	context->Draw(1200, 0);

	// END PART 5

	// TODO: PART 3 STEP 5
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

	// TODO: PART 2 STEP 9b
	context->PSSetShader(ps, 0, 0);

	// TODO: PART 2 STEP 9c
	context->IASetInputLayout(inputLayout);

	// TODO: PART 2 STEP 9d
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// TODO: PART 2 STEP 10
	context->Draw(36, 0);

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
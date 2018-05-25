//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include <vector>
#include "XTime.h"

#include "DDSTextureLoader.h"

#include "Barrel.h"
#include "greendragon.h"
#include "StoneHenge.h"
#include "3d_planet.h"

#define TID_POLLMOUSE 100
#define MOUSE_POLL_DELAY 5

#define NUM_OF_MATRICIES 13
#define NUM_OF_VBUFFERS 7
#define NUM_OF_IBUFFERS 7
#define NUM_OF_DVIEWS 11
#define NUM_OF_SSTATES 1
#define NUM_OF_NORMMAPS 3
#define MAX_LIGHTS 3

using namespace std;

#include <d3d11.h>
#pragma comment(lib, "d3d11");

#include <DirectXMath.h>

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "Trivial_GS.csh"
#include "SkyBox_PS.csh"
#include "SkyBox_VS.csh"

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

	D3D11_VIEWPORT rtt_viewPort;
	D3D11_VIEWPORT arrayOfViewports[2];

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
	ID3D11ShaderResourceView* normMaps[NUM_OF_NORMMAPS];

	ID3D11InputLayout* inputLayout;
	ID3D11InputLayout* skyBox_inputLayout;

	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;
	ID3D11GeometryShader* gs;
	ID3D11VertexShader* skyBox_vs;
	ID3D11PixelShader* skyBox_ps;

	ID3D11Buffer* cBufferVS;
	D3D11_BUFFER_DESC cBufferVSDesc = {};
	ID3D11Buffer* cBufferPS;
	D3D11_BUFFER_DESC cBufferPSDesc = {};
	ID3D11Buffer* cBufferGS;
	D3D11_BUFFER_DESC cBufferGSDesc = {};
	XTime time;
	float timer = 0.0;
	float cameraTimer = 0.0;

	float cooldown = 0.0;

	bool enableSecondViewport = false;

	DirectX::XMMATRIX worldMatricies[NUM_OF_MATRICIES];
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX cameraMatrix;
	DirectX::XMMATRIX second_cameraMatrix;
	POINT tempPointDown = { MININT, MININT };
	DirectX::XMMATRIX projectionMatrix;

	struct MATERIAL
	{
		DirectX::XMFLOAT4 emissive;
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;

		int specularPower;

		DirectX::XMFLOAT3 padding;
	};

	struct LIGHTS
	{
		DirectX::XMFLOAT4 lightDirection;
		DirectX::XMFLOAT4 lightPosition;
		DirectX::XMFLOAT4 lightColor;

		int lightType;
		int enabled;

		DirectX::XMFLOAT2 padding;
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
		MATERIAL materialProperties;

		LIGHTS lights[MAX_LIGHTS];
		DirectX::XMFLOAT4 globalAmbientColor;
		DirectX::XMFLOAT4 eyePosition;

		int hasSecondTexture;
		int hasNormMap;
		int isRTT;

		float padding;
	};

	struct SEND_TO_GS
	{
		DirectX::XMMATRIX second_viewMatrix;

		int isBarrels;

		int splitScreenEnabled;

		DirectX::XMFLOAT3 padding;
	};

	SEND_TO_VS toVSShader;
	SEND_TO_PS toPSShader;
	SEND_TO_GS toGSShader;

public:
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoords;
		DirectX::XMFLOAT3 tangent;
	};

	struct SKYBOX_VERTEX
	{
		DirectX::XMFLOAT3 pos;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run(); 
	bool ShutDown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

	void CheckKeyboardInput();
	bool LoadObjHeader(const OBJ_VERT* data, int dataSize, const unsigned int* indicies, int indiciesSize, int vBuffer, int iBuffer);
	void DEMO_APP::CalculateTangent(SIMPLE_VERTEX* data, unsigned int dataSize, short* indicies, unsigned int indiciesSize);
	int LoadObjFile(const char* path, unsigned int vBuffer, unsigned int iBuffer);
	void MakeVertexBuffer(ID3D11Buffer** buffer, int size, SIMPLE_VERTEX* data);
	void MakeVertexBuffer(ID3D11Buffer** buffer, int size, SKYBOX_VERTEX* data);
	void MakeIndexBuffer(ID3D11Buffer** buffer, int size, short* data);
};

DEMO_APP* myApp = nullptr;

void DEMO_APP::CheckKeyboardInput()
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

		if (enableSecondViewport)
		{
			if (GetAsyncKeyState(0x49)) // 'I'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.05f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(tempM, second_cameraMatrix);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(0x4A)) // 'J'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(-0.05f, 0.0f, 0.0f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(tempM, second_cameraMatrix);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(0x4B)) // 'K'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.05f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(tempM, second_cameraMatrix);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(0x4C)) // 'L'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.05f, 0.0f, 0.0f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(tempM, second_cameraMatrix);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(0x55)) // 'U'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, 0.05f, 0.0f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(second_cameraMatrix, tempM);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(0x4F)) // 'O'
			{
				DirectX::XMMATRIX tempM = DirectX::XMMatrixTranslation(0.0f, -0.05f, 0.0f);

				second_cameraMatrix = DirectX::XMMatrixMultiply(second_cameraMatrix, tempM);

				worldMatricies[11].r[3] = second_cameraMatrix.r[3];

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(VK_LEFT)) // 'LEFT'
			{
				DirectX::XMVECTOR tempVector = second_cameraMatrix.r[3];;

				second_cameraMatrix.r[3] = DirectX::XMLoadFloat4(&DirectX::XMFLOAT4(0, 0, 0, 1));

				second_cameraMatrix = DirectX::XMMatrixMultiply(second_cameraMatrix, DirectX::XMMatrixRotationY(-0.05f));

				second_cameraMatrix.r[3] = tempVector;

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(VK_RIGHT)) // 'RIGHT'
			{
				DirectX::XMVECTOR tempVector = second_cameraMatrix.r[3];;

				second_cameraMatrix.r[3] = DirectX::XMLoadFloat4(&DirectX::XMFLOAT4(0, 0, 0, 1));

				second_cameraMatrix = DirectX::XMMatrixMultiply(second_cameraMatrix, DirectX::XMMatrixRotationY(0.05f));

				second_cameraMatrix.r[3] = tempVector;

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(VK_UP)) // 'UP'
			{
				DirectX::XMVECTOR tempVector = second_cameraMatrix.r[3];;

				second_cameraMatrix.r[3] = DirectX::XMLoadFloat4(&DirectX::XMFLOAT4(0, 0, 0, 1));

				second_cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(-0.05f), second_cameraMatrix);

				second_cameraMatrix.r[3] = tempVector;

				cameraTimer = 0.0f;
			}
			if (GetAsyncKeyState(VK_DOWN)) // 'DOWN'
			{
				DirectX::XMVECTOR tempVector = second_cameraMatrix.r[3];;

				second_cameraMatrix.r[3] = DirectX::XMLoadFloat4(&DirectX::XMFLOAT4(0, 0, 0, 1));

				second_cameraMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(0.05f), second_cameraMatrix);

				second_cameraMatrix.r[3] = tempVector;

				cameraTimer = 0.0f;
			}
		}
	}
	else
		cameraTimer += time.SmoothDelta();

	if (cooldown >= 0.1f)
	{
		if (GetAsyncKeyState(0x32) && cooldown >= 0.1f) // '2'
		{
			if (enableSecondViewport)
			{
				enableSecondViewport = false;

				cooldown = 0.0f;
			}
			else
			{
				enableSecondViewport = true;

				cooldown = 0.0f;
			}
		}
	}
	else
		cooldown += time.SmoothDelta();
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

void DEMO_APP::CalculateTangent(SIMPLE_VERTEX* data, unsigned int dataSize, short* indicies, unsigned int indiciesSize)
{
	std::vector<DirectX::XMFLOAT3> tempTangent;
	DirectX::XMFLOAT3 tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float tcU1, tcV1, tcU2, tcV2;

	float vecX, vecY, vecZ;

	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < dataSize; i+=3)
	{
		vecX = data[indicies[i]].pos.x - data[indicies[i + 2]].pos.x;
		vecY = data[indicies[i]].pos.y - data[indicies[i + 2]].pos.y;
		vecZ = data[indicies[i]].pos.z - data[indicies[i + 2]].pos.z;
		edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		vecX = data[indicies[i + 2]].pos.x - data[indicies[i + 1]].pos.x;
		vecY = data[indicies[i + 2]].pos.y - data[indicies[i + 1]].pos.y;
		vecZ = data[indicies[i + 2]].pos.z - data[indicies[i + 1]].pos.z;
		edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		tcU1 = data[indicies[i]].texCoords.x - data[indicies[i + 2]].texCoords.x;
		tcV1 = data[indicies[i]].texCoords.y - data[indicies[i + 2]].texCoords.y;

		tcU2 = data[indicies[i + 2]].texCoords.x - data[indicies[i + 1]].texCoords.x;
		tcV2 = data[indicies[i + 2]].texCoords.y - data[indicies[i + 1]].texCoords.y;

		tangent.x = (tcV1 * DirectX::XMVectorGetX(edge1) - tcV2 * DirectX::XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.y = (tcV1 * DirectX::XMVectorGetY(edge1) - tcV2 * DirectX::XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.z = (tcV1 * DirectX::XMVectorGetZ(edge1) - tcV2 * DirectX::XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
	
		tempTangent.push_back(tangent);
	}

	DirectX::XMVECTOR tangentSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	unsigned int facesUsing = 0;
	float tX, tY, tZ;

	for (unsigned int i = 0; i < indiciesSize; i+=3)
	{
		for (unsigned int j = 0; j < dataSize; j+=3)
		{
			if (indicies[j] == i || indicies[j + 1] == i || indicies[j + 2] == i)
			{
				tX = DirectX::XMVectorGetX(tangentSum) + tempTangent[j / 3].x;
				tY = DirectX::XMVectorGetY(tangentSum) + tempTangent[j / 3].y;
				tZ = DirectX::XMVectorGetZ(tangentSum) + tempTangent[j / 3].z;

				tangentSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f);

				++facesUsing;
			}
		}

		DirectX::XMVectorSetX(tangentSum, DirectX::XMVectorGetX(tangentSum) / facesUsing);
		DirectX::XMVectorSetY(tangentSum, DirectX::XMVectorGetY(tangentSum) / facesUsing);
		DirectX::XMVectorSetZ(tangentSum, DirectX::XMVectorGetZ(tangentSum) / facesUsing);

		tangentSum = DirectX::XMVector3Normalize(tangentSum);

		data[i].tangent.x = DirectX::XMVectorGetX(tangentSum);
		data[i].tangent.y = DirectX::XMVectorGetY(tangentSum);
		data[i].tangent.z = DirectX::XMVectorGetZ(tangentSum);

		tangentSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		facesUsing = 0;
	}
}

int DEMO_APP::LoadObjFile(const char* path, unsigned int vBuffer, unsigned int iBuffer)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<DirectX::XMFLOAT3> tempVertices;
	std::vector<DirectX::XMFLOAT2> tempUVs;
	std::vector<DirectX::XMFLOAT3> tempNormals;

	unsigned int index = 0;

	FILE * file = fopen(path, "r");
	if (file == NULL)
	{
		return false;
	}

	while (1)
	{
		char line[128];

		int res = fscanf(file, "%s", line);
		if (res == EOF)
			break;

		if (strcmp(line, "v") == 0)
		{
			DirectX::XMFLOAT3 vert;
			fscanf(file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			tempVertices.push_back(vert);
		}
		else if (strcmp(line, "vt") == 0)
		{
			DirectX::XMFLOAT2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y *= -1;
			tempUVs.push_back(uv);
		}
		else if (strcmp(line, "vn") == 0)
		{
			DirectX::XMFLOAT3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			tempNormals.push_back(normal);
		}
		else if (strcmp(line, "f") == 0)
		{
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	SIMPLE_VERTEX* tempVertsArray = new SIMPLE_VERTEX[vertexIndices.size()];
	short* tempIndicesArray;

	std::vector<short> tempIndicesVector;

	int vectorSize = 0;

	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
		unsigned int vertexIndex = vertexIndices[i];
		DirectX::XMFLOAT3 vert = tempVertices[vertexIndex - 1];
	
		unsigned int uvIndex = uvIndices[i];
		DirectX::XMFLOAT2 uv = tempUVs[uvIndex - 1];
	
		unsigned int normalIndex = normalIndices[i];
		DirectX::XMFLOAT3 normal = tempNormals[normalIndex - 1];

		bool result = false;

		for (unsigned int j = 0; j < vectorSize; ++j)
		{
			if (vert.x == tempVertsArray[j].pos.x &&
				vert.y == tempVertsArray[j].pos.y &&
				vert.z == tempVertsArray[j].pos.z &&
				uv.x == tempVertsArray[j].texCoords.x &&
				uv.y == tempVertsArray[j].texCoords.y &&
				normal.x == tempVertsArray[j].normal.x &&
				normal.y == tempVertsArray[j].normal.y &&
				normal.z == tempVertsArray[j].normal.z)
			{
				result = true;
				tempIndicesVector.push_back(j);
			}
		}
	
		if (result == false)
		{
			tempIndicesVector.push_back(index);
			++index;

			tempVertsArray[vectorSize].pos = vert;
			tempVertsArray[vectorSize].texCoords = uv;
			tempVertsArray[vectorSize].normal = normal;

			++vectorSize;
		}
	}

	tempIndicesArray = new short[tempIndicesVector.size()];

	for (unsigned int i = 0; i < tempIndicesVector.size(); ++i)
	{
		tempIndicesArray[i] = tempIndicesVector[i];
	}

	CalculateTangent(tempVertsArray, tempVertices.size(), tempIndicesArray, tempIndicesVector.size());

	MakeVertexBuffer(&vBuffers[vBuffer], vectorSize, tempVertsArray);
	MakeIndexBuffer(&iBuffers[iBuffer], tempIndicesVector.size(), tempIndicesArray);

	delete[] tempVertsArray;
	delete[] tempIndicesArray;

	return tempIndicesVector.size();
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

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW,
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
	HRESULT result = device->CreateTexture2D(&depthDesc, NULL, &depthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthViewDesc.Texture2D.MipSlice = 0;
	result = device->CreateDepthStencilView(depthBuffer, &depthViewDesc, &dsv);

	rtt_viewPort.Width = 500;
	rtt_viewPort.Height = 500;
	rtt_viewPort.MinDepth = 0.0f;
	rtt_viewPort.MaxDepth = 1.0f;
	rtt_viewPort.TopLeftX = 0;
	rtt_viewPort.TopLeftY = 0;

	arrayOfViewports[0].Width = backBuffer_width;
	arrayOfViewports[0].Height = backBuffer_height;
	arrayOfViewports[0].MinDepth = 0.0f;
	arrayOfViewports[0].MaxDepth = 1.0f;
	arrayOfViewports[0].TopLeftX = 0;
	arrayOfViewports[0].TopLeftY = 0;

	arrayOfViewports[1].Width = backBuffer_width / 2;
	arrayOfViewports[1].Height = backBuffer_height;
	arrayOfViewports[1].MinDepth = 0.0f;
	arrayOfViewports[1].MaxDepth = 1.0f;
	arrayOfViewports[1].TopLeftX = backBuffer_width / 2;
	arrayOfViewports[1].TopLeftY = 0;

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
	result = device->CreateTexture2D(&rttDesc, NULL, &rtt_texture);

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
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f)},

		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, -0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, -0.25f)},

		{ DirectX::XMFLOAT3(-0.25f, -0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(0.25f, -0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f)},
		{ DirectX::XMFLOAT3(-0.25f, 0.25f, 0.25f)},
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

	MakeVertexBuffer(&vBuffers[0], ARRAYSIZE(cubeMapVerts), cubeMapVerts);
	MakeIndexBuffer(&iBuffers[0], ARRAYSIZE(cubeMapTriangleVerts), cubeMapTriangleVerts);
	result = CreateDDSTextureFromFile(device, L"mountain_skybox.dds", nullptr, &diffuseViews[9]);
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

	int num = LoadObjFile("Barrel.obj", 5, 5);

	MakeVertexBuffer(&vBuffers[1], ARRAYSIZE(cubeVerts), cubeVerts);
	MakeIndexBuffer(&iBuffers[1], ARRAYSIZE(triangleVerts), triangleVerts);
	result = CreateDDSTextureFromFile(device, L"greendragon.dds", nullptr, &diffuseViews[1]);

	LoadObjHeader(Barrel_data, ARRAYSIZE(Barrel_data), Barrel_indicies, ARRAYSIZE(Barrel_indicies), 2, 2);
	result = CreateDDSTextureFromFile(device, L"Barrel.dds", nullptr, &diffuseViews[2]);

	LoadObjHeader(StoneHenge_data, ARRAYSIZE(StoneHenge_data), StoneHenge_indicies, ARRAYSIZE(StoneHenge_indicies), 3, 3);
	result = CreateDDSTextureFromFile(device, L"StoneHenge.dds", nullptr, &diffuseViews[3]);

	num = LoadObjFile("3d_planet.obj", 4, 4);
	result = CreateDDSTextureFromFile(device, L"earthmap1k.dds", nullptr, &diffuseViews[4]);
	result = CreateDDSTextureFromFile(device, L"earthcloudmap.dds", nullptr, &diffuseViews[5]);
	result = CreateDDSTextureFromFile(device, L"moonmap1k.dds", nullptr, &diffuseViews[6]);
	result = CreateDDSTextureFromFile(device, L"moonbump1k.dds", nullptr, &normMaps[0]);
	result = CreateDDSTextureFromFile(device, L"sunmap.dds", nullptr, &diffuseViews[7]);
	
	num = LoadObjFile("satellite.obj", 6, 6);
	result = CreateDDSTextureFromFile(device, L"RT_2D_Satellite_Diffuse.dds", nullptr, &diffuseViews[10]);
	result = CreateDDSTextureFromFile(device, L"RT_2d_Satellite_Normal.dds", nullptr, &normMaps[2]);

	result = CreateDDSTextureFromFile(device, L"stone01.dds", nullptr, &diffuseViews[8]);
	result = CreateDDSTextureFromFile(device, L"bump01.dds", nullptr, &normMaps[1]);

	result = device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	result = device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);
	result = device->CreateGeometryShader(Trivial_GS, sizeof(Trivial_GS), NULL, &gs);
	result = device->CreateVertexShader(SkyBox_VS, sizeof(SkyBox_VS), NULL, &skyBox_vs);
	result = device->CreatePixelShader(SkyBox_PS, sizeof(SkyBox_PS), NULL, &skyBox_ps);

	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "SV_INSTANCEID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	D3D11_INPUT_ELEMENT_DESC skyBox_vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	result = device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);
	result = device->CreateInputLayout(skyBox_vLayout, ARRAYSIZE(skyBox_vLayout), SkyBox_VS, sizeof(SkyBox_VS), &skyBox_inputLayout);

	ZeroMemory(&cBufferVSDesc, sizeof(cBufferVS));
	cBufferVSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferVSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferVSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferVSDesc.ByteWidth = sizeof(SEND_TO_VS);
	result = device->CreateBuffer(&cBufferVSDesc, NULL, &cBufferVS);

	ZeroMemory(&cBufferPSDesc, sizeof(cBufferPS));
	cBufferPSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferPSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferPSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferPSDesc.ByteWidth = sizeof(SEND_TO_PS);
	result = device->CreateBuffer(&cBufferPSDesc, NULL, &cBufferPS);

	ZeroMemory(&cBufferGSDesc, sizeof(cBufferGS));
	cBufferGSDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferGSDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferGSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferGSDesc.ByteWidth = sizeof(SEND_TO_GS);
	result = device->CreateBuffer(&cBufferGSDesc, NULL, &cBufferGS);

	for (int i = 0; i < NUM_OF_MATRICIES; ++i)
	{
		worldMatricies[i] = DirectX::XMMatrixIdentity();
	}

	DirectX::XMVECTOR cameraPosition = { 0.0f, -2.0f, 5.0f };
	DirectX::XMVECTOR cameraLook = { 0.0f, 2.5f, 15.0f };
	DirectX::XMVECTOR cameraUp = { 0.0f, 1.0f, 0.0f };

	cameraMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraLook, cameraUp);

	second_cameraMatrix = cameraMatrix;

	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), (backBuffer_width) / (backBuffer_height), 0.1f, 5000.0f);

	rtt_viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	rtt_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), 100 / 100, 0.1f, 100.0f);
	
	worldMatricies[3] = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);
	worldMatricies[3].r[3] = cameraMatrix.r[3];
	worldMatricies[11] = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);
	worldMatricies[11].r[3] = second_cameraMatrix.r[3];
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	time.Signal();

	CheckKeyboardInput();

	context->OMSetRenderTargets(1, &rtv, dsv);

	context->RSSetViewports(1, &arrayOfViewports[0]);

	FLOAT arr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(rtv, arr);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);
	//
	// DRAW THE SKYBOX
	//
	toVSShader.worldMatrix = worldMatricies[3];
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	toVSShader.viewMatrix = viewMatrix;
	toVSShader.projectionMatrix = projectionMatrix;

	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->VSSetConstantBuffers(0, 1, &cBufferVS);

	UINT stride = sizeof(SKYBOX_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vBuffers[0], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[0], DXGI_FORMAT_R16_UINT, offset);

	context->VSSetShader(skyBox_vs, 0, 0);
	context->PSSetShader(skyBox_ps, 0, 0);
	context->GSSetShader(nullptr, 0, 0);

	context->PSSetShaderResources(1, 1, &diffuseViews[0]);
	context->PSSetSamplers(0, 1, &samplerStates[0]);

	context->IASetInputLayout(skyBox_inputLayout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(36, 0, 0);

	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	if (enableSecondViewport)
	{
		context->RSSetViewports(1, &arrayOfViewports[1]);

		toVSShader.worldMatrix = worldMatricies[11];
		viewMatrix = DirectX::XMMatrixInverse(nullptr, second_cameraMatrix);
		toVSShader.viewMatrix = viewMatrix;

		toGSShader.splitScreenEnabled = 1;

		context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
		context->Unmap(cBufferVS, NULL);

		context->Map(cBufferGS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &toGSShader, sizeof(toGSShader));
		context->Unmap(cBufferGS, NULL);

		context->DrawIndexed(36, 0, 0);

		context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);
	}
	else
	{
		toGSShader.splitScreenEnabled = 0;

		context->Map(cBufferGS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &toGSShader, sizeof(toGSShader));
		context->Unmap(cBufferGS, NULL);
	}
	//
	// DRAW THE FLAT CUBE RTT
	//
	worldMatricies[0] = DirectX::XMMatrixTranslation(-5.0f, 2.5f, 0.0f);
	worldMatricies[0] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-90)), worldMatricies[0]);
	worldMatricies[0] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(90)), worldMatricies[0]);
	worldMatricies[0] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(10.0f, 1.0f, 10.0f), worldMatricies[0]);
	toVSShader.worldMatrix = worldMatricies[0];
	viewMatrix = DirectX::XMMatrixInverse(nullptr, cameraMatrix);
	toVSShader.viewMatrix = viewMatrix;
	toVSShader.projectionMatrix = projectionMatrix;

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.specularPower = 64;
	toPSShader.lights[0].lightDirection = DirectX::XMFLOAT4(-0.3f, -cos(timer * 0.4f), -cos(timer * 0.4f), 1.0f);
	toPSShader.lights[0].lightPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.lights[0].lightColor = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	toPSShader.lights[0].lightType = 0;
	toPSShader.lights[0].enabled = 1;
	toPSShader.lights[1].lightDirection = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	toPSShader.lights[1].lightPosition = DirectX::XMFLOAT4(cos(timer) - 1.0f, 0.25f, -0.25f, 1.0f);
	toPSShader.lights[1].lightColor = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	toPSShader.lights[1].lightType = 1;
	toPSShader.lights[1].enabled = 1;
	toPSShader.lights[2].lightDirection = DirectX::XMFLOAT4(0.0f, -1.0f, 0.6f, 1.0f);
	toPSShader.lights[2].lightPosition = DirectX::XMFLOAT4(cos(timer * 1.5f), 2.0f, -cos(timer * 0.5f), 1.0f);
	toPSShader.lights[2].lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.lights[2].lightType = 2;
	toPSShader.lights[2].enabled = 1;
	toPSShader.globalAmbientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	toPSShader.eyePosition = DirectX::XMFLOAT4(viewMatrix.r[3].m128_f32[0], viewMatrix.r[3].m128_f32[1], viewMatrix.r[3].m128_f32[2], viewMatrix.r[3].m128_f32[3]);
	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	if (enableSecondViewport)
	{
		if (arrayOfViewports[0].Width != backBuffer_width / 2)
		{
			arrayOfViewports[0].Width = backBuffer_width / 2;
		
			projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), (backBuffer_width / 2) / (backBuffer_height), 0.1f, 5000.0f);
		}
		context->RSSetViewports(2, &arrayOfViewports[0]);
	}
	else
	{
		if (arrayOfViewports[0].Width != backBuffer_width)
		{
			arrayOfViewports[0].Width = backBuffer_width;
		
			projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), (backBuffer_width) / (backBuffer_height), 0.1f, 5000.0f);		
		}
		context->RSSetViewports(1, &arrayOfViewports[0]);
	}
	
	toGSShader.second_viewMatrix = DirectX::XMMatrixInverse(nullptr, second_cameraMatrix);
	toGSShader.isBarrels = 0;

	timer += time.SmoothDelta();

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->Map(cBufferGS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toGSShader, sizeof(toGSShader));
	context->Unmap(cBufferGS, NULL);

	context->PSSetConstantBuffers(1, 1, &cBufferPS);
	context->GSSetConstantBuffers(3, 1, &cBufferVS);
	context->GSSetConstantBuffers(4, 1, &cBufferGS);

	stride = sizeof(SIMPLE_VERTEX);
	context->IASetVertexBuffers(0, 1, &vBuffers[1], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[1], DXGI_FORMAT_R16_UINT, offset);

	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);
	context->GSSetShader(gs, 0, 0);

	context->PSSetShaderResources(0, 1, &rtt_shaderResource);

	context->IASetInputLayout(inputLayout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(36, 0, 0);

	context->PSSetShaderResources(0, 1, pSRV);
	//
	// DRAW THE FLAT CUBE
	//
	worldMatricies[8] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	worldMatricies[8] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(10.0f, 1.0f, 10.0f), worldMatricies[8]);
	toVSShader.worldMatrix = worldMatricies[8];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.specularPower = 64;
	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->PSSetShader(ps, 0, 0);

	context->PSSetShaderResources(0, 1, &diffuseViews[1]);

	context->DrawIndexed(36, 0, 0);
	//
	// DRAW THE BARRELS
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[2], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[2], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShader(ps, 0, 0);
	context->GSSetShader(gs, 0, 0);

	context->PSSetShaderResources(0, 1, &diffuseViews[2]);

	worldMatricies[9] = DirectX::XMMatrixTranslation(2.0f, 0.25f, 2.0f);
	worldMatricies[9] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.05f, 0.05f, 0.05f), worldMatricies[9]);
	toVSShader.worldMatrix = worldMatricies[9];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	toPSShader.materialProperties.specularPower = 12.8;
	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	toGSShader.isBarrels = 1;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->Map(cBufferGS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toGSShader, sizeof(toGSShader));
	context->Unmap(cBufferGS, NULL);

	context->IASetVertexBuffers(0, 1, &vBuffers[5], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[5], DXGI_FORMAT_R16_UINT, offset);

	context->DrawIndexed(612, 0, 0);
	//
	// DRAW THE SUN
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[4], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[4], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShaderResources(0, 1, &diffuseViews[7]);

	worldMatricies[5] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 150.0f);
	worldMatricies[5] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[5]);
	worldMatricies[5] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f), worldMatricies[5]);
	toVSShader.worldMatrix = worldMatricies[5];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.95f, 0.6f, 0.25f, 0.7f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.specularPower = 10;
	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	toGSShader.isBarrels = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->Map(cBufferGS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toGSShader, sizeof(toGSShader));
	context->Unmap(cBufferGS, NULL);

	context->DrawIndexed(7140, 0, 0);
	//
	// DRAW THE EARTH
	//
	context->PSSetShaderResources(0, 1, &diffuseViews[4]);
	context->PSSetShaderResources(2, 1, &diffuseViews[5]);

	worldMatricies[6] = DirectX::XMMatrixTranslation(worldMatricies[5].r[3].m128_f32[0] + 0.0f, worldMatricies[5].r[3].m128_f32[1] + 0.0f, worldMatricies[5].r[3].m128_f32[2] + 0.0f);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(100.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(timer * 0.15f)), worldMatricies[6]);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[6]);
	worldMatricies[6] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.03f, 0.03f, 0.03f), worldMatricies[6]);
	toVSShader.worldMatrix = worldMatricies[6];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.specularPower = 5;
	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 1;
	toPSShader.isRTT = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(7140, 0, 0);
	//
	// DRAW THE MOON
	//
	context->PSSetShaderResources(0, 1, &diffuseViews[6]);
	context->PSSetShaderResources(2, 1, pSRV);
	context->PSSetShaderResources(3, 1, &normMaps[0]);

	worldMatricies[7] = DirectX::XMMatrixTranslation(worldMatricies[6].r[3].m128_f32[0] + 0.0f, worldMatricies[6].r[3].m128_f32[1] + 0.0f, worldMatricies[6].r[3].m128_f32[2] + 0.0f);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-25.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(timer * 0.7f)), worldMatricies[7]);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.65f), worldMatricies[7]);
	worldMatricies[7] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f), worldMatricies[7]);
	toVSShader.worldMatrix = worldMatricies[7];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	toPSShader.materialProperties.specularPower = 10;
	toPSShader.hasNormMap = 1;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(7140, 0, 0);
	//
	// DRAW SATILLITE
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[6], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[6], DXGI_FORMAT_R16_UINT, offset);
	
	context->PSSetShaderResources(0, 1, &diffuseViews[10]);
	context->PSSetShaderResources(3, 1, &normMaps[2]);
	
	worldMatricies[12] = DirectX::XMMatrixTranslation(5.0f, 4.0f, 0.0f);
	worldMatricies[12] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-195)), DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.5f), worldMatricies[12]));
	worldMatricies[12] = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f), worldMatricies[12]);
	toVSShader.worldMatrix = worldMatricies[12];
	
	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	toPSShader.materialProperties.specularPower = 10;
	toPSShader.hasNormMap = 1;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;
	
	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);
	
	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);
	
	context->DrawIndexedInstanced(7236, 3, 0, 0, 0);
	//
	// DRAW EXAMPLE CUBE 1
	//
	context->IASetVertexBuffers(0, 1, &vBuffers[1], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[1], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShaderResources(0, 1, &diffuseViews[8]);
	context->PSSetShaderResources(3, 1, &normMaps[1]);

	worldMatricies[1] = DirectX::XMMatrixTranslation(-1.5f, 1.0f, 0.0f);
	toVSShader.worldMatrix = worldMatricies[1];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	toPSShader.materialProperties.specularPower = 32;
	toPSShader.hasNormMap = 1;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(36, 0, 0);
	//
	// DRAW EXAMPLE CUBE 2
	//
	worldMatricies[10] = DirectX::XMMatrixTranslation(-1.0f, 1.0f, 0.0f);
	toVSShader.worldMatrix = worldMatricies[10];

	toPSShader.materialProperties.emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	toPSShader.materialProperties.ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	toPSShader.materialProperties.specularPower = 64;
	toPSShader.hasNormMap = 1;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 0;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

	context->DrawIndexed(36, 0, 0);
	//
	// DRAW STONEHENGE ONTO A TEXTURE TO BE RENDERED
	//
	context->OMSetRenderTargets(1, &rtt_rtv, rtt_dsv);

	context->RSSetViewports(1, &rtt_viewPort);

	arr[0] = 1.0f;
	arr[1] = 1.0f;
	arr[2] = 1.0f;
	context->ClearRenderTargetView(rtt_rtv, arr);
	context->ClearDepthStencilView(rtt_dsv, D3D11_CLEAR_DEPTH, 1.0f, 1.0f);

	context->IASetVertexBuffers(0, 1, &vBuffers[3], &stride, &offset);
	context->IASetIndexBuffer(iBuffers[3], DXGI_FORMAT_R16_UINT, offset);

	context->PSSetShaderResources(0, 1, &diffuseViews[3]);
	context->PSSetShaderResources(3, 1, pSRV);

	worldMatricies[4] = DirectX::XMMatrixTranslation(0.0f, -15.0f, 15.0f);
	worldMatricies[4] = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(timer * 0.3f), worldMatricies[4]);
	toVSShader.worldMatrix = worldMatricies[4];
	toVSShader.viewMatrix = rtt_viewMatrix;
	toVSShader.projectionMatrix = rtt_projectionMatrix;

	toPSShader.hasNormMap = 0;
	toPSShader.hasSecondTexture = 0;
	toPSShader.isRTT = 1;

	context->Map(cBufferVS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toVSShader, sizeof(toVSShader));
	context->Unmap(cBufferVS, NULL);

	context->Map(cBufferPS, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &toPSShader, sizeof(toPSShader));
	context->Unmap(cBufferPS, NULL);

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
	swapChain->Release();
	depthBuffer->Release();
	rtv->Release();
	dsv->Release();
	context->Release();
	cBufferVS->Release();
	cBufferPS->Release();
	cBufferGS->Release();
	ps->Release();
	vs->Release();
	gs->Release();
	skyBox_ps->Release();
	skyBox_vs->Release();
	inputLayout->Release();
	skyBox_inputLayout->Release();
	rtt_depthBuffer->Release();
	rtt_dsv->Release();
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
	for (unsigned int i = 0; i < NUM_OF_NORMMAPS; ++i)
	{
		normMaps[i]->Release();
	}

	//ID3D11Debug* debug;
	//device->QueryInterface(IID_PPV_ARGS(&debug));
	//debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

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

			if (myApp->enableSecondViewport)
			{
				myApp->arrayOfViewports[0].Width = myApp->backBuffer_width / 2;
				myApp->arrayOfViewports[0].Height = myApp->backBuffer_height;
				myApp->arrayOfViewports[0].MinDepth = 0.0f;
				myApp->arrayOfViewports[0].MaxDepth = 1.0f;
				myApp->arrayOfViewports[0].TopLeftX = 0;
				myApp->arrayOfViewports[0].TopLeftY = 0;

				myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), (myApp->backBuffer_width / 2) / (myApp->backBuffer_height), 0.1f, 5000.0f);
			}
			else
			{
				myApp->arrayOfViewports[0].Width = myApp->backBuffer_width;
				myApp->arrayOfViewports[0].Height = myApp->backBuffer_height;
				myApp->arrayOfViewports[0].MinDepth = 0.0f;
				myApp->arrayOfViewports[0].MaxDepth = 1.0f;
				myApp->arrayOfViewports[0].TopLeftX = 0;
				myApp->arrayOfViewports[0].TopLeftY = 0;

				myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), (myApp->backBuffer_width) / (myApp->backBuffer_height), 0.1f, 5000.0f);
			}

			myApp->arrayOfViewports[1].Width = myApp->backBuffer_width / 2;
			myApp->arrayOfViewports[1].Height = myApp->backBuffer_height;
			myApp->arrayOfViewports[1].MinDepth = 0.0f;
			myApp->arrayOfViewports[1].MaxDepth = 1.0f;
			myApp->arrayOfViewports[1].TopLeftX = myApp->backBuffer_width / 2;
			myApp->arrayOfViewports[1].TopLeftY = 0;

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
		if (myApp->fov >= 45 && myApp->fov <= 120)
			myApp->fov += -(GET_WHEEL_DELTA_WPARAM(wParam) / 120);

		if (myApp->fov < 45)
			myApp->fov = 45;
		else if (myApp->fov > 120)
			myApp->fov = 120;

		if (myApp->enableSecondViewport)
			myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), (myApp->backBuffer_width / 2) / (myApp->backBuffer_height), 0.1f, 5000.0f);
		else
			myApp->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(myApp->fov), (myApp->backBuffer_width) / (myApp->backBuffer_height), 0.1f, 5000.0f);
	}
					break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//
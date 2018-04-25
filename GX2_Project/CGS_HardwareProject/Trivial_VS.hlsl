#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;
};

//struct LIGHT
//{
//	float3 lightDirection;
//	float4 lightPosition;
//	float4 ambientColor;
//};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;

	//LIGHT light[3];
	float3 lightDirection;
	float4 lightPosition;
	float4 ambientColor;
};

cbuffer SCENE : register(b1)
{
	//NF3D_LIGHT_OMNIDIRECTIONAL light;
	//float4x4 viewMatrix;
	//float4x4 projectionMatrix;
}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	sendToRasterizer.worldPos = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.worldPos, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);
	
	sendToRasterizer.normal = mul(fromVertexBuffer.normal, (float3x3)worldMatrix);
	sendToRasterizer.normal = normalize(sendToRasterizer.normal);

	sendToRasterizer.texOut = fromVertexBuffer.tex;

	return sendToRasterizer;
}
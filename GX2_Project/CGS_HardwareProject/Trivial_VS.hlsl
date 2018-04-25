#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
<<<<<<< HEAD
	float3 coordinate : POSITION;
=======
	float4 coordinate : POSITION;
	float4 color : COLOR;
>>>>>>> parent of 0f18ab0... Working on textures
	//float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	//float4 color : COLOR;
};

struct OUTPUT_VERTEX
{
<<<<<<< HEAD
	float2 texOut : TEXCOORD0;
=======
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
>>>>>>> parent of 0f18ab0... Working on textures
	//float3 normal : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	//float4 colorOut : COLOR;
};

cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	
	//float3 lightDirection;
	//float4 ambientColor;
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);
	
	//sendToRasterizer.normal = mul(fromVertexBuffer.normal, (float3x3)worldMatrix);
	//sendToRasterizer.normal = normalize(sendToRasterizer.normal);

	sendToRasterizer.texOut = fromVertexBuffer.tex;

	return sendToRasterizer;
}
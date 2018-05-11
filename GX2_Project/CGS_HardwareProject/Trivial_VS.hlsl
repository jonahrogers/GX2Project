#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;

	uint instanceId : SV_INSTANCEID;
	//uint primitiveId : SV_PRIMITIVEID;
};

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	uint instanceId : SV_INSTANCEID;
	//uint primitiveId : SV_PRIMITIVEID;
};

struct LIGHTS
{
	float3 lightDirection;
	float padding;
	float4 lightPosition;
	float4 lightColor;
};

cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	sendToRasterizer.worldPos = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);
	
	if (fromVertexBuffer.instanceId % 2 == 0)
	{
	sendToRasterizer.worldPos.z -= fromVertexBuffer.instanceId / 2;
	}
	else if (fromVertexBuffer.instanceId % 2 == 1)
	{
		sendToRasterizer.worldPos.z -= (fromVertexBuffer.instanceId - 1) / 2;

		sendToRasterizer.worldPos.x -= 1;
	}
	
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.worldPos, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);
	
	sendToRasterizer.normal = mul(fromVertexBuffer.normal, (float3x3)worldMatrix);
	sendToRasterizer.normal = normalize(sendToRasterizer.normal);

	sendToRasterizer.texOut = fromVertexBuffer.tex;

	sendToRasterizer.instanceId = fromVertexBuffer.instanceId;

	return sendToRasterizer;
}
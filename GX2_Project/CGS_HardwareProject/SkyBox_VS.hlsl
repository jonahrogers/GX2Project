#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
};

struct OUTPUT_VERTEX_SKYBOX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 texOut : TEXCOORD0;
};

cbuffer THIS_IS_OTHER_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

OUTPUT_VERTEX_SKYBOX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX_SKYBOX sendToRasterizer;

	sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);

	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);

	sendToRasterizer.texOut = fromVertexBuffer.coordinate.xyz;

	return sendToRasterizer;
}
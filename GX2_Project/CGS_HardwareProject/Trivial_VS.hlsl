#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float2 tex : TEXCOORD0;
	//float4 color : COLOR;
	//float3 normal : NORMAL;
};

struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	//float4 colorOut : COLOR;
	//float3 normal : NORMAL;
};

//struct NF3D_LIGHT_OMNIDIRECTIONAL
//{
//	float4 Diffuse;
//	float4 Ambient;
//	float3 Position;
//	float3 Attitude;
//	float Range;
//	int BindSlot;
//};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 rgba;

	//float4 constantColor;
	//float2 constantOffset;
	//float2 padding;
};

//cbuffer SCENE : register(b1)
//{
//	NF3D_LIGHT_OMNIDIRECTIONAL light;
//	float4x4 viewMatrix;
//	float4x4 projectionMatrix;
//}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	//sendToRasterizer.projectedCoordinate.xy = fromVertexBuffer.coordinate.xy;

	sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);

	//sendToRasterizer.UVsOut = mul(fromVertexBuffer.UVs, worldMatrix);
	//sendToRasterizer.UVsOut = mul(sendToRasterizer.UVsOut, viewMatrix);
	//sendToRasterizer.UVsOut = mul(sendToRasterizer.UVsOut, projectionMatrix);
	
	//sendToRasterizer.normal = mul(fromVertexBuffer.normal, (float3x3)worldMatrix);
	//sendToRasterizer.normal = normalize(sendToRasterizer.normal);
	//sendToRasterizer.normal = fromVertexBuffer.coordinate.xyz;

	// TODO : PART 4 STEP 4
	//sendToRasterizer.projectedCoordinate.xy += constantOffset;

	// TODO : PART 3 STEP 7
	sendToRasterizer.texOut = fromVertexBuffer.tex;
	// END PART 3

	return sendToRasterizer;
}
#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	//float3 normal : NORMAL;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
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

	//float4 constantColor;
	//float2 constantOffset;
	//float2 padding;
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
	//sendToRasterizer.projectedCoordinate.w = 1;

	//sendToRasterizer.projectedCoordinate.xy = fromVertexBuffer.coordinate.xy;

	sendToRasterizer.projectedCoordinate = mul(fromVertexBuffer.coordinate, worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);
	
	//sendToRasterizer.normal = mul(fromVertexBuffer.normal, (float3x3)worldMatrix);
	//sendToRasterizer.normal = normalize(sendToRasterizer.normal);

	// TODO : PART 4 STEP 4
	//sendToRasterizer.projectedCoordinate.xy += constantOffset;

	// TODO : PART 3 STEP 7
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	// END PART 3

	return sendToRasterizer;
}
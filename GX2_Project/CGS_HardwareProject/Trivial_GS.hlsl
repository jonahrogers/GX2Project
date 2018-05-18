#pragma pack_matrix(row_major)

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;

	uint instanceId : SV_INSTANCEID;
};

struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;
};

cbuffer THIS_IS_VRAM : register(b3)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

[maxvertexcount(3)]
void main(triangle OUTPUT_VERTEX input[3], inout TriangleStream<OUTPUT_GEOMETRY> OutputStream)
{
	OUTPUT_GEOMETRY output = (OUTPUT_GEOMETRY)0;

	for (int i = 0; i < 3; ++i)
	{
		output.projectedCoordinate = input[i].projectedCoordinate;
		output.normal = input[i].normal;
		output.texOut = input[i].texOut;
		output.worldPos = input[i].worldPos;
		output.tangent = input[i].tangent;

		if (input[i].instanceId % 2 == 0)
		{
			output.worldPos.z -= input[i].instanceId / 2;
		}
		else if (input[i].instanceId % 2 == 1)
		{
			output.worldPos.z -= (input[i].instanceId - 1) / 2;

			output.worldPos.x -= 1;
		}

		output.projectedCoordinate = mul(output.worldPos, viewMatrix);
		output.projectedCoordinate = mul(output.projectedCoordinate, projectionMatrix);

		OutputStream.Append(output);
	}
}
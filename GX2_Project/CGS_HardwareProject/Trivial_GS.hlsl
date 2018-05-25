#pragma pack_matrix(row_major)

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;
};

struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;

	uint viewPort : SV_VIEWPORTARRAYINDEX;
};

cbuffer THIS_IS_VRAM : register(b3)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

cbuffer SECOND_VIEWMATRIX : register(b4)
{
	float4x4 second_viewMatrix;

	int isBarrels;
	int splitScreenEnabled;

	float2 padding;
}

[maxvertexcount(33)]

void main(triangle OUTPUT_VERTEX input[3], inout TriangleStream<OUTPUT_GEOMETRY> OutputStream)
{
	OUTPUT_GEOMETRY output = (OUTPUT_GEOMETRY)0;

	int numOfViewPorts = 0;

	if (!isBarrels)
	{
		if (splitScreenEnabled)
			numOfViewPorts = 2;
		else
			numOfViewPorts = 1;

		for (int i = 0; i < numOfViewPorts; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				output.projectedCoordinate = input[j].projectedCoordinate;
				output.normal = input[j].normal;
				output.texOut = input[j].texOut;
				output.worldPos = input[j].worldPos;
				output.tangent = input[j].tangent;

				if (i == 1)
				{
					output.projectedCoordinate = mul(output.worldPos, second_viewMatrix);
					output.projectedCoordinate = mul(output.projectedCoordinate, projectionMatrix);
				}

				output.viewPort = i;

				OutputStream.Append(output);
			}

		OutputStream.RestartStrip();
		}
	}
	else
	{
		for (int i = 0; i < 10; ++i)
		{
			if (i < 1)
			{
				if (splitScreenEnabled)
					numOfViewPorts = 2;
				else
					numOfViewPorts = 1;
	
				for (int k = 0; k < numOfViewPorts; ++k)
				{
					for (int j = 0; j < 3; ++j)
					{
						output.projectedCoordinate = input[j].projectedCoordinate;
						output.normal = input[j].normal;
						output.texOut = input[j].texOut;
						output.worldPos = input[j].worldPos;
						output.tangent = input[j].tangent;
	
						if (k == 1)
						{
							output.projectedCoordinate = mul(output.worldPos, second_viewMatrix);
							output.projectedCoordinate = mul(output.projectedCoordinate, projectionMatrix);
						}
	
						output.viewPort = k;
	
						OutputStream.Append(output);
					}
					
					OutputStream.RestartStrip();
				}
			}
			else
			{
				for (int j = 0; j < 3; ++j)
				{
					output.projectedCoordinate = input[j].projectedCoordinate;
					output.normal = input[j].normal;
					output.texOut = input[j].texOut;
					output.worldPos = input[j].worldPos;
					output.tangent = input[j].tangent;

					if (i % 2 == 0)
					{
						output.worldPos.z -= i / 2;

						output.projectedCoordinate = mul(output.worldPos, viewMatrix);
						output.projectedCoordinate = mul(output.projectedCoordinate, projectionMatrix);

						output.viewPort = 0;
					}
					else if (i % 2 == 1 && splitScreenEnabled)
					{
						output.worldPos.z -= (i - 1) / 2;

						output.worldPos.x -= 1;

						output.projectedCoordinate = mul(output.worldPos, second_viewMatrix);
						output.projectedCoordinate = mul(output.projectedCoordinate, projectionMatrix);

						output.viewPort = 1;
					}

					OutputStream.Append(output);
				}

				OutputStream.RestartStrip();
			}
		}
	}
}
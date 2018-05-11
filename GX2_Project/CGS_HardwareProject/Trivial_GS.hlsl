
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	uint instanceId : SV_INSTANCEID;
};

struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	//uint primitiveId : SV_PRIMITIVEID;
};

[maxvertexcount(3)]
void main(triangle OUTPUT_VERTEX input[3], inout TriangleStream<OUTPUT_GEOMETRY> OutputStream)
{
	OUTPUT_GEOMETRY output = (OUTPUT_GEOMETRY)0;

		for (int i = 0; i < 3; ++i)
		{
			//output = input[i];
			output.projectedCoordinate = input[i].projectedCoordinate;
			output.normal = input[i].normal;
			output.texOut = input[i].texOut;
			output.worldPos = input[i].worldPos;
			//output.primitiveId = primitiveId;

			output.worldPos.x -= input[i].instanceId;
			output.projectedCoordinate.x -= input[i].instanceId;

			/*if (input[i].instanceId == 1)
			{
				output.worldPos.x -= input[i].instanceId;
				output.worldPos.y += input[i].instanceId;
				output.projectedCoordinate.x -= input[i].instanceId;
				output.projectedCoordinate.y += input[i].instanceId;
			}
			else if (input[i].instanceId == 2)
			{
				output.worldPos.x += input[i].instanceId;
				output.worldPos.y += input[i].instanceId;
				output.projectedCoordinate.x += input[i].instanceId;
				output.projectedCoordinate.y += input[i].instanceId;
			}
			else if (input[i].instanceId == 3)
			{
				output.worldPos.x -= input[i].instanceId;
				output.worldPos.y -= input[i].instanceId;
				output.projectedCoordinate.x -= input[i].instanceId;
				output.projectedCoordinate.y -= input[i].instanceId;
			}
			else if (input[i].instanceId == 4)
			{
				output.worldPos.x += input[i].instanceId;
				output.worldPos.y -= input[i].instanceId;
				output.projectedCoordinate.x += input[i].instanceId;
				output.projectedCoordinate.y -= input[i].instanceId;
			}*/

			OutputStream.Append(output);
		}

		/*OutputStream.RestartStrip();

		for (int i = 0; i < 3; ++i)
		{
			output = input[i];

			output.worldPos.x += primitiveId;
			output.worldPos.y += primitiveId;

			OutputStream.Append(output);
		}

	OutputStream.RestartStrip();*/
}
struct OUTPUT_VERTEX_SKYBOX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 texOut : TEXCOORD0;
};

textureCUBE cubeTexture : register(t1);

SamplerState cubeFilter : register(s1);


float4 main(OUTPUT_VERTEX_SKYBOX vertex) : SV_TARGET
{
	float4 color = cubeTexture.Sample(cubeFilter, vertex.texOut);

	return color;
}
//struct OUTPUT_VERTEX
//{
//	float4 coordinate : SV_POSITION;
//	float2 tex : TEXCOORD0;
//	//float4 color : COLOR;
//	//float3 normal : NORMAL;
//};

texture2D baseTexture : register(t0);

SamplerState baseFilter : register(s0);

//cbuffer LightBuffer
//{
//	float4 ambientColor;
//	float4 diffuseColor;
//	float3 lightDirection;
//	float padding;
//};

float4 main( /*OUTPUT_VERTEX input*/float2 tex : TEXCOORD0) : SV_TARGET
{
	/*float3 lightDir;
	float lightIntensity;
	float4 color;

	color = ambientColor;

	lightDir = -lightDirection;

	lightIntensity = saturate(dot(colorFromRasterizer.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		color += (diffuseColor * lightIntensity);
	}

	color = saturate(color);*/

	return baseTexture.Sample( baseFilter, tex );
}
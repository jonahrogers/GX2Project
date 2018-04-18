//struct OUTPUT_VERTEX
//{
//	float4 coordinate : SV_POSITION;
//	float2 tex : TEXCOORD0;
//	//float4 color : COLOR;
//	//float3 normal : NORMAL;
//};

texture2D baseTexture : register(t0);

SamplerState baseFilter : register(s0);

cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;

	float3 lightDirection;

	float4 ambientColor;

	//float4 constantColor;
	//float2 constantOffset;
	//float2 padding;
};

float4 main( /*OUTPUT_VERTEX input*/float2 tex : TEXCOORD0/*, float3 normal : NORMAL*/) : SV_TARGET
{
	//float3 lightDir;
	//float lightIntensity;
	//float4 color;
	//
	//color = baseTexture.Sample(baseFilter, tex);
	//
	//lightDir = -lightDirection;
	//
	//lightIntensity = saturate(dot(normal, lightDir));
	//
	//if (lightIntensity > 0.0f)
	//{
	//	color += (ambientColor * lightIntensity);
	//}
	//
	//color = saturate(color);

	return baseTexture.Sample(baseFilter, tex);;
}
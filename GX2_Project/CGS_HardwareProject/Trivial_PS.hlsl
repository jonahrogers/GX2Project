struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;
};

texture2D baseTexture : register(t0);

SamplerState baseFilter : register(s0);

//struct LIGHT
//{
//	float3 lightDirection;
//	float4 lightPosition;
//	float4 ambientColor;
//};

cbuffer THIS_IS_VRAM : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;

	//LIGHT light[3];
	float3 lightDirection;
	float4 lightPosition;
	float4 ambientColor;
}

float4 main(OUTPUT_VERTEX vertex ) : SV_TARGET
{
	float4 color = baseTexture.Sample(baseFilter, vertex.texOut);
	
	float4 ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);

	//for directional light
	float3 lightDir = normalize(-lightDirection);
	float lightRatioDir = saturate(dot(lightDir, vertex.normal));
	lightRatioDir = saturate(lightRatioDir + ambient);
	color = lightRatioDir * ambientColor * color;

	// for point light
	//float3 lightPoint = (normalize(light[1].lightPosition - vertex.worldPos)).xyz;
	//float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
	//float attenuationPoint = 1.0 - saturate(length(light[1].lightPosition.xyz - vertex.worldPos.xyz) / 2.5f);
	//lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));// + ambient);
	//float4 pointColor = lightRatioPoint * light[1].ambientColor;// *color;

	//for spot light
	//float3 lightSpot = normalize(light[2].lightPosition.xyz - vertex.worldPos.xyz);
	//float surfaceRatio = saturate(dot(-lightSpot, lightDir));
	//float spotFactor = (surfaceRatio > 20) ? 1 : 0;
	//float lightRatioSpot = saturate(dot(lightSpot, vertex.normal));
	//float attenuationSpot = 1.0 - saturate((0.6f - surfaceRatio) / (0.6f - 0.5f));
	//lightRatioSpot = saturate((lightRatioSpot * attenuationSpot));// +ambient);
	//float4 spotColor = surfaceRatio * lightRatioSpot * light[2].ambientColor;// * color;

	color = saturate(color);

	return color;
}
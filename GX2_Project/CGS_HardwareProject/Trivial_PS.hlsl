struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;
};

texture2D baseTexture : register(t0);

SamplerState baseFilter : register(s0);

struct LIGHTS
{
	float3 lightDirection;
	float padding;
	float4 lightPosition;
	float4 lightColor;
};

cbuffer THIS_IS_VRAM2 : register(b1)
{
	LIGHTS lights[3];
	float4 ambientColor;
}

float4 main(OUTPUT_VERTEX vertex ) : SV_TARGET
{
	float4 color = baseTexture.Sample(baseFilter, vertex.texOut);

	//for directional light
	float3 lightDir = -normalize(lights[0].lightDirection);
	float lightRatioDir = saturate(dot(lightDir, vertex.normal));
	lightRatioDir = saturate(lightRatioDir + ambientColor);
	float4 dirColor = lightRatioDir * lights[0].lightColor;
	
	// for point light
	float3 lightPoint = (normalize(lights[1].lightPosition - vertex.worldPos)).xyz;
	float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
	float attenuationPoint = 1.0f - saturate(length(lights[1].lightPosition.xyz - vertex.worldPos.xyz) / 2.5f);
	lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));
	float4 pointColor = lightRatioPoint * lights[1].lightColor;

	//for spot light
	float3 lightSpot = normalize(lights[2].lightPosition.xyz - vertex.worldPos.xyz);
	float surfaceRatio = saturate(dot(-lightSpot, lights[2].lightDirection));
	float spotFactor;
	if (surfaceRatio > -1)
		spotFactor = 1;
	else
		spotFactor = 0;
	float lightRatioSpot = saturate(dot(lightSpot, vertex.normal));
	float attenuationSpot = 1.0f - saturate((1.0f - surfaceRatio) / (1.0f - 0.9f));
	lightRatioSpot = saturate((lightRatioSpot * attenuationSpot));
	float4 spotColor = spotFactor * lightRatioSpot * lights[2].lightColor;

	color = (dirColor + pointColor + spotColor) * color;

	color = saturate(color);

	return color;
}
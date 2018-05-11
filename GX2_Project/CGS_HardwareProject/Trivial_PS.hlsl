struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;
	//uint primitiveId : SV_PRIMITIVEID;
};

texture2D baseTexture : register(t0);
texture2D otherTexture : register(t3);

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
	//float4 emissiveValue;
}

float4 main(OUTPUT_GEOMETRY vertex) : SV_TARGET
{
	float4 color;
	float4 dirColor = 0;
	float4 pointColor = 0;
	float4 spotColor = 0;

	uint width = 0;
	uint height = 0;

	otherTexture.GetDimensions(width, height);

	if (width == 0 && height == 0)
	{
		color = baseTexture.Sample(baseFilter, vertex.texOut);
	}
	else
	{
		color = baseTexture.Sample(baseFilter, vertex.texOut);

		float4 color2 = otherTexture.Sample(baseFilter, vertex.texOut);

		color = color * color2 * 2.0f;
	}

	//for directional light
	if (lights[0].lightColor.w > 0.0f)
	{
		float3 lightDir = -normalize(lights[0].lightDirection);
		float lightRatioDir = saturate(dot(lightDir, vertex.normal));
		lightRatioDir = saturate(lightRatioDir + ambientColor);
		dirColor = lightRatioDir * lights[0].lightColor;
	}

	// for point light
	if (lights[1].lightColor.w > 0.0f)
	{
		float3 lightPoint = (normalize(lights[1].lightPosition - vertex.worldPos)).xyz;
		float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
		float attenuationPoint = 1.0f - saturate(length(lights[1].lightPosition.xyz - vertex.worldPos.xyz) / 5.0f);
		lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));
		pointColor = lightRatioPoint * lights[1].lightColor;
	}

	//for spot light
	if (lights[2].lightColor.w > 0.0f)
	{
		float3 lightSpot = normalize(lights[2].lightPosition.xyz - vertex.worldPos.xyz);
		float surfaceRatio = saturate(dot(-lightSpot, lights[2].lightDirection));
		float spotFactor;
		if (surfaceRatio > -1)
			spotFactor = 1;
		else
			spotFactor = 0;
		float lightRatioSpot = saturate(dot(lightSpot, vertex.normal));
		float attenuationSpot = 0.2f - saturate((0.2f - surfaceRatio) / (0.2f - 0.1f));
		lightRatioSpot = saturate((lightRatioSpot * attenuationSpot));
		spotColor = spotFactor * lightRatioSpot * lights[2].lightColor;
	}

	if (lights[0].lightColor.w == 0 && lights[1].lightColor.w == 0 && lights[2].lightColor.w == 0)
		return color;
	else
	{
			color = (dirColor + pointColor + spotColor) * color;

		color = saturate(color);

		return color;
	}
}
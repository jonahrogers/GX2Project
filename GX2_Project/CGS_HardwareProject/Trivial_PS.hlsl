struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;
};

texture2D baseTexture : register(t0);
texture2D otherTexture : register(t3);

texture2D normalMapTexture : register(t4);

SamplerState baseFilter : register(s0);

struct DIRLIGHTS
{
	float3 lightDirection;
	float padding;
	float4 lightColor;
};

struct POINTLIGHTS
{
	float4 lightPosition;
	float4 lightColor;
};

struct SPOTLIGHTS
{
	float3 lightDirection;
	float padding;
	float4 lightPosition;
	float4 lightColor;
};

cbuffer THIS_IS_VRAM2 : register(b1)
{
	DIRLIGHTS dirLights[1];
	POINTLIGHTS pointLights[1];
	SPOTLIGHTS spotLights[1];
	float4 ambientColor;
	//float4 emissiveValue;

	float hasSecondTexture;
	float hasNormMap;
	float isRTT;

	float padding;
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

	if (hasSecondTexture == 0)
	{
		color = baseTexture.Sample(baseFilter, vertex.texOut);
	}
	else if (hasSecondTexture == 1)
	{
		color = baseTexture.Sample(baseFilter, vertex.texOut);

		float4 color2 = otherTexture.Sample(baseFilter, vertex.texOut);

		color = color * color2 * 2.0f;
	}

	if (hasNormMap == 1)
	{
		float4 normalMap = normalMapTexture.Sample(baseFilter, vertex.texOut);
	
		normalMap = (2.0f * normalMap) - 1.0f;
	
		vertex.tangent = normalize(vertex.tangent - dot(vertex.tangent, vertex.normal) * vertex.normal);
	
		float3 biTangent = cross(vertex.normal, vertex.tangent);
	
		float3x3 texSpace = float3x3(vertex.tangent, biTangent, vertex.normal);
	
		vertex.normal = normalize(mul(normalMap, texSpace));
	}

	//for directional light
	if (!isRTT)
	{
		float3 lightDir = -normalize(dirLights[0].lightDirection);
		float lightRatioDir = saturate(dot(lightDir, vertex.normal));
		lightRatioDir = saturate(lightRatioDir + ambientColor);
		dirColor = lightRatioDir * dirLights[0].lightColor;
	//}

	// for point light
	//if (lights[1].lightColor.w > 0.0f)
	//{
		float3 lightPoint = (normalize(pointLights[0].lightPosition - vertex.worldPos)).xyz;
		float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
		float attenuationPoint = 1.0f - saturate(length(pointLights[0].lightPosition.xyz - vertex.worldPos.xyz) / 10.0f);
		attenuationPoint *= attenuationPoint;
		lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));
		pointColor = lightRatioPoint * pointLights[0].lightColor;
	//}

	//for spot light
	//if (lights[2].lightColor.w > 0.0f)
	//{
		float3 lightSpot = normalize(spotLights[0].lightPosition.xyz - vertex.worldPos.xyz);
		float surfaceRatio = saturate(dot(-lightSpot, spotLights[0].lightDirection));
		float spotFactor;
		if (surfaceRatio > -1)
			spotFactor = 1;
		else
			spotFactor = 0;
		float lightRatioSpot = saturate(dot(lightSpot, vertex.normal));
		float attenuationSpot = 1.0f - saturate((1.0f - surfaceRatio) / (1.0f - 0.9f));
		lightRatioSpot = saturate((lightRatioSpot * attenuationSpot));
		spotColor = spotFactor * lightRatioSpot * spotLights[0].lightColor;

		color = (dirColor + pointColor + spotColor) * color;

		color = saturate(color);

		return color;
	}
	else
	{
		//float3 lightPoint = (normalize(pointLights[0].lightPosition - vertex.worldPos)).xyz;
		//float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
		//float attenuationPoint = 1.0f - saturate(length(pointLights[0].lightPosition.xyz - vertex.worldPos.xyz) / 20.0f);
		////attenuationPoint = attenuationPoint * attenuationPoint;
		//lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));
		//pointColor = saturate(lightRatioPoint + ambientColor) * pointLights[0].lightColor;

		return saturate(color);
	}

	//if (lights[0].lightColor.w == 0 && lights[1].lightColor.w == 0 && lights[2].lightColor.w == 0)
	//	return saturate(color);
	//else
	//{
	//	color = (dirColor + pointColor + spotColor) * color;
	//	
	//	color = saturate(color);
	//
	//	return color;
	//}
}
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2 

#define MAX_LIGHTS 3

struct OUTPUT_GEOMETRY
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normal : NORMAL;
	float2 texOut : TEXCOORD0;
	float4 worldPos : POSITION;

	float3 tangent : TANGENT;

	uint renderTarget : SV_VIEWPORTARRAYINDEX;
};

texture2D baseTexture : register(t0);
texture2D otherTexture : register(t2);

texture2D normalMapTexture : register(t3);

SamplerState baseFilter : register(s0);

struct MATERIAL
{
	float4 emissive;
	float4 ambient;
	float4 diffuse;
	float4 specular;

	int specularPower;

	float3 padding;
};

struct LIGHTS
{
	float4 lightDirection;
	float4 lightPosition;
	float4 lightColor;

	int lightType;
	int enabled;

	float2 padding;
};

cbuffer THIS_IS_VRAM2 : register(b1)
{
	MATERIAL materialProperties;

	LIGHTS lights[MAX_LIGHTS];
	float4 globalAmbientColor;
	float4 eyePosition;

	int hasSecondTexture;
	int hasNormMap;
	int isRTT;

	float padding;
}

float4 main(OUTPUT_GEOMETRY vertex) : SV_TARGET
{
	float4 color = 0;

	float4 totalEmissive = 0;
	float4 totalAmbient = 0;
	float4 totalDiffuse = 0;
	float4 totalSpecular = 0;

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

	if (!isRTT)
	{
		for (unsigned int i = 0; i < MAX_LIGHTS; ++i)
		{
			if (lights[i].enabled)
			{
				switch (lights[i].lightType)
				{
				case DIRECTIONAL_LIGHT: // for directional light
				{
					float3 lightDir = -normalize(lights[i].lightDirection.xyz);
					float lightRatioDir = saturate(dot(lightDir, vertex.normal));
					lightRatioDir = saturate(lightRatioDir + globalAmbientColor);
					totalDiffuse += lightRatioDir * lights[i].lightColor;

					float3 viewDir = -normalize(eyePosition.xyz);
					float3 reflect = -lightDir - 2 * vertex.normal * dot(-lightDir, vertex.normal);
					float specRatio = saturate(dot(reflect, viewDir));
					totalSpecular += lights[i].lightColor * pow(specRatio, materialProperties.specularPower);
				}
				break;
				case POINT_LIGHT: // for point light
				{
					float3 lightPoint = (normalize(lights[i].lightPosition - vertex.worldPos)).xyz;
					float lightRatioPoint = saturate(dot(lightPoint, vertex.normal));
					float attenuationPoint = 1.0f - saturate(length(lights[i].lightPosition.xyz - vertex.worldPos.xyz) / 2.5f);
					attenuationPoint *= attenuationPoint;
					lightRatioPoint = saturate((lightRatioPoint * attenuationPoint));
					totalDiffuse += lightRatioPoint * lights[i].lightColor;

					float3 viewDir = -normalize(eyePosition.xyz);
					float3 reflect = normalize(lights[i].lightDirection.xyz) - 2 * vertex.normal * dot(normalize(lights[i].lightDirection.xyz), vertex.normal);
					float specRatio = saturate(dot(reflect, viewDir));
					totalSpecular += lightRatioPoint * lights[i].lightColor * pow(specRatio, materialProperties.specularPower);
				}
				break;
				case SPOT_LIGHT: // for spot light
				{
					float3 lightSpot = normalize(lights[i].lightPosition.xyz - vertex.worldPos.xyz);
					float surfaceRatio = saturate(dot(-lightSpot, lights[i].lightDirection));
					float spotFactor;
					if (surfaceRatio > -1)
						spotFactor = 1;
					else
						spotFactor = 0;
					float lightRatioSpot = saturate(dot(lightSpot, vertex.normal));
					float attenuationSpot = 1.0f - saturate((1.0f - surfaceRatio) / (1.0f - 0.9f));
					lightRatioSpot = saturate((lightRatioSpot * attenuationSpot));
					totalDiffuse += spotFactor * lightRatioSpot * lights[i].lightColor;

					float3 viewDir = -normalize(eyePosition.xyz);
					float3 reflect = normalize(lights[i].lightDirection.xyz) - 2 * vertex.normal * dot(normalize(lights[i].lightDirection.xyz), vertex.normal);
					float specRatio = saturate(dot(reflect, viewDir));
					totalSpecular += spotFactor * lights[i].lightColor * pow(specRatio, materialProperties.specularPower);
				}
				break;
				default:
					break;
				}
			}
		}

		totalEmissive = float4(materialProperties.emissive.x * materialProperties.emissive.w, materialProperties.emissive.y * materialProperties.emissive.w, materialProperties.emissive.z * materialProperties.emissive.w, materialProperties.emissive.w);
		totalAmbient = materialProperties.ambient * globalAmbientColor;
		totalDiffuse = materialProperties.diffuse * totalDiffuse;
		totalSpecular = materialProperties.specular * totalSpecular;

		color = (totalEmissive + totalAmbient + totalDiffuse + totalSpecular) * color;

		color = saturate(color);

		return color;
	}
	else
	{
		return saturate(color);
	}
}
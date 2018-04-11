//struct INPUT_PIXEL
//{
//	float4 coordinate : SV_POSITION;
//	float3 normal : NORMAL;
//};
//
//cbuffer LightBuffer
//{
//	float4 ambientColor;
//	float4 diffuseColor;
//	float3 lightDirection;
//	float padding;
//};

float4 main(/*INPUT_PIXEL*/float4 colorFromRasterizer : COLOR) : SV_TARGET
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

	return colorFromRasterizer;
}
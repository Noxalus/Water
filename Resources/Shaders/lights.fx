shared matrix WorldMatrix;
shared matrix ViewMatrix;
shared matrix ProjectionMatrix;
shared Texture2D ShaderTexture;
shared float4 AmbientColor;
shared float4 DiffuseColor;
shared float3 LightDirection;
shared float3 CameraPosition;

struct VertexInputType
{
	float3 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal 	  : NORMAL;
};

struct PixelInputType
{
	float4 Position      : POSITION;
	float2 UV  		     : TEXCOORD0;
	float3 Normal        : TEXCOORD1;
	float3 ViewDirection : TEXCOORD2;
};

// Sample
sampler2D  SampleType = sampler_state
{
	Texture = ShaderTexture;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

PixelInputType VertexMain(VertexInputType input)
{
	PixelInputType output;
	float4 worldPosition;

	// Change the position vector to be 4 units for proper matrix calculations.
	float4 position = float4(input.Position, 1.0f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.Position = mul(position, WorldMatrix);
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.UV = input.UV;

	// Calculate the normal vector against the world matrix only.
	output.Normal = mul(input.Normal, (float3x3)WorldMatrix);

	// Normalize the normal vector.
	output.Normal = normalize(output.Normal);

	// Calculate the position of the vertex in the world.
	worldPosition = mul(input.Position, WorldMatrix);

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	output.ViewDirection = CameraPosition.xyz - worldPosition.xyz;

	// Normalize the viewing direction vector.
	output.ViewDirection = normalize(output.ViewDirection);

	return output;
}

float4 PixelMain(PixelInputType input) : COLOR0
{
	float4 color;
	float4 textureColor;
	float lightIntensity;
	float3 invertedLightDirection;
	float3 reflection;
	float4 specular;
	float specularPower = 128.0f;

	textureColor = tex2D(SampleType, input.UV);
	color = AmbientColor;

	// Invert the light direction for calculations.
	invertedLightDirection = -LightDirection;

	specular = float4(0, 0, 0, 0);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.Normal, invertedLightDirection));

	if (lightIntensity > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		color += (DiffuseColor * lightIntensity);

		// Saturate the final light color.
		color = saturate(color);

		// Specular computation
		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
		reflection = normalize(2 * lightIntensity * input.Normal - invertedLightDirection);
		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		specular = pow(saturate(dot(reflection, input.ViewDirection)), specularPower);
	}

	color = color * textureColor;

	// Add the specular component last to the output color.
	//color = saturate(color + specular);

	return color;
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader = compile ps_3_0 PixelMain();
	}
}
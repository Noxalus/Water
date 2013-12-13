// Globals
shared matrix WorldMatrix;
shared matrix ViewMatrix;
shared matrix ProjectionMatrix;
shared Texture2D ShaderTexture;
shared float4 AmbientColor;
shared float4 DiffuseColor;
shared float3 LightDirection;
shared float4 ClipPlane;

// Samples
sampler2D SampleType = sampler_state
{
	Texture = ShaderTexture;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

// Typedefs
struct VertexInputType
{
	float3 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal 	  : NORMAL;
};

struct PixelInputType
{
	float4 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal 	  : TEXCOORD1;
	float  Clip       : TEXCOORD2;
};

// Vertex shader
PixelInputType VertexMain(VertexInputType input)
{
	PixelInputType output;

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

	// Set the clipping plane.
	output.Clip = dot(mul(input.Position, WorldMatrix), ClipPlane);

	return output;
}

// Pixel shader
float4 PixelMain(PixelInputType input) : COLOR0
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	// Sample the texture pixel at this location.
	textureColor = tex2D(SampleType, input.UV);

	// Set the default output color to the ambient light value for all pixels.
	color = AmbientColor;

	// Invert the light direction for calculations.
	lightDir = -LightDirection;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.Normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		color += (DiffuseColor * lightIntensity);
	}

	// Saturate the final light color.
	color = saturate(color);

	// Multiply the texture pixel and the input color to get the final result.
	color = color * textureColor;
	
	return textureColor;
}

// Technique
technique RefractionTechnique
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader = compile ps_3_0 PixelMain();
	}
}
shared matrix WorldMatrix;
shared matrix ViewMatrix;
shared matrix ProjectionMatrix;
shared matrix ReflectionMatrix;
shared Texture2D NormalTexture;
shared Texture2D ReflectedTexture;

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
	float4 ReflectionPosition : TEXCOORD1;
};

struct PixelOutput
{

};

sampler2D  MapSampler = sampler_state
{
	Texture = ReflectedTexture;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

PixelInputType VertexMain(VertexInputType input)
{
	PixelInputType output;

	matrix reflectProjectWorld;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	float4 position = float4(input.Position, 1.0f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.Position = mul(position, WorldMatrix);
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	
	// Store the texture coordinates for the pixel shader.
	output.UV = input.UV;
	
	// Create the reflection projection world matrix.
	reflectProjectWorld = mul(ReflectionMatrix, ProjectionMatrix);
	reflectProjectWorld = mul(WorldMatrix, reflectProjectWorld);

	// Calculate the input position against the reflectProjectWorld matrix.
	output.ReflectionPosition = mul(input.Position, reflectProjectWorld);
	
	return output;
}

float4 PixelMain(PixelInputType input) : COLOR0
{
	float4 textureColor;
	float2 reflectTexCoord;
	float4 reflectionColor;
	float4 color;

	textureColor = tex2D(MapSampler, input.UV);

	// Calculate the projected reflection texture coordinates.
	reflectTexCoord.x = input.ReflectionPosition.x / input.ReflectionPosition.w / 2.0f + 0.5f;
	reflectTexCoord.y = -input.ReflectionPosition.y / input.ReflectionPosition.w / 2.0f + 0.5f;

	// Sample the texture pixels from the textures using the updated texture coordinates.
	reflectionColor = tex2D(MapSampler, reflectTexCoord);

	// Combine the reflection and refraction results for the final color.
	//color = lerp(reflectionColor, refractionColor, 0.6f);

	color = lerp(textureColor, reflectionColor, 0.15f);

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
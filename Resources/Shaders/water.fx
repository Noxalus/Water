#define LIGHTS_NUMBER 1

shared float4x4 WorldViewProj;
shared Texture2D NormalTexture;

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
	float3 Normal     : TEXCOORD1;
};

struct PixelOutput
{

};

sampler2D  MapSampler = sampler_state
{
	Texture = NormalTexture;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

PixelInputType VertexMain(VertexInputType input)
{
	PixelInputType output;

	output.Position = mul(float4(input.Position, 1.0f), WorldViewProj);
	output.UV = input.UV;
	output.Normal = input.Normal;

	return output;
}

float4 PixelMain(PixelInputType input) : COLOR0
{
	return tex2D(MapSampler, input.UV);
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader = compile ps_3_0 PixelMain();
	}
}
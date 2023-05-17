#pragma target 3.0
#include "noiseSimplex.cginc"

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix Rotation;

	float4 Color;
	float4 noiseColor;
}

struct VS_INPUT
{
    float4 Pos   : POSITION;
    float4 Norm  : NORMAL;
};

struct PS_INPUT
{
    float4 Pos   : SV_POSITION;
	float noise : FLOAT0;
	float dot : FLOAT1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	float noise = snoise(input.Pos);
	output.Pos = input.Pos;
	output.Pos = mul(output.Pos, Rotation);
	input.Norm = mul(input.Norm, Rotation);
	output.Pos = mul(output.Pos, World);
	output.dot = 1.0f - dot(normalize(output.Pos), input.Norm);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.noise = noise;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	return input.dot * 0.5 * (1 + input.noise) * Color;
}

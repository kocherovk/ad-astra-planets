#pragma target 3.0
#include "noiseSimplex.cginc"

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix Rotation;
	float4 Color;
}

struct VS_INPUT
{
    float4 Pos   : POSITION;
    float4 Norm  : NORMAL;
};

struct PS_INPUT
{
    float4 Pos   : SV_POSITION;
	float4 Color : COLOR;
	float  noise : FLOAT;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	output.Pos = mul(input.Pos, Rotation);
	input.Norm = mul(input.Norm, Rotation);
	output.Pos = mul(output.Pos, World);
	output.Color = (1.0f - dot(normalize(output.Pos), input.Norm)) * Color;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.noise = snoise(input.Pos);

    return output;
}

//--------------------------------------------------------------------------------------
// PixelShader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float noise = input.noise / 1.5;
	return float4(noise, noise, noise, 0.0f) + Color;
}


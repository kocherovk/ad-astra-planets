
cbuffer ConstantBuffer : register(b0)
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
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT) 0;

	output.Pos = mul(input.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	return output;
}

//--------------------------------------------------------------------------------------
// PixelShader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}


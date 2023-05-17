
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

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
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	float4 positionInSpace = mul(input.Pos, World);
	output.Pos = mul(positionInSpace, View);
	output.Pos = mul(output.Pos, Projection);
	output.Color = (1.0f - dot(normalize(positionInSpace), input.Norm)) * Color;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	return input.Color;
}

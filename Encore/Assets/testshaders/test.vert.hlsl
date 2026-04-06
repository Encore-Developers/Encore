cbuffer UniformBlock : register(b0, space1)
{
    float4x4 MatrixTransform : packoffset(c0);
};

struct Input
{
	float4 Position : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
	Output output;
    output.Position = mul(MatrixTransform, input.Position);
	output.TexCoord = input.TexCoord;
    return output;
}

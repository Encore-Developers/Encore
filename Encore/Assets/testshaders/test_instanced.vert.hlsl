cbuffer UniformBlock : register(b0, space1)
{
    float4x4 MatrixTransform;
	float2 uvOffset;
};

struct Input
{
	float3 Position : INPUT0;
	float2 TexCoord : INPUT1;
    uint VertexIndex : SV_VertexID;
};

struct InstanceInput {
    float2 Position : INSTANCEINPUT2;
	float2 Scale : INSTANCEINPUT3;
};

struct Output
{
    float2 TexCoord : OUTPUT0;
    float4 Position : SV_Position;
};

Output main(Input input, InstanceInput instance)
{
	Output output;
    output.Position = mul(MatrixTransform, float4((input.Position * float3(instance.Scale.x, instance.Scale.y, 1)) + float3(instance.Position.x, 0, instance.Position.y), 1.0));
	output.TexCoord = input.TexCoord + uvOffset;
    return output;
}

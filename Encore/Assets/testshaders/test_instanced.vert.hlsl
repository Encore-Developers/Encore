#include <camera_uniform.inc.hlsl>
#include <generic_mesh.inc.hlsl>


struct InstanceInput {
    float2 Position : INSTANCEINPUT2;
	float2 Scale : INSTANCEINPUT3;
};

struct Output
{
    float2 TexCoord : OUTPUT0;
    float4 Position : SV_Position;
};

Output main(GenericMesh input, InstanceInput instance)
{
	Output output;
    output.Position = mul(MatrixTransform, float4((input.Position * float3(instance.Scale.x, instance.Scale.y, 1)) + float3(instance.Position.x, 0, instance.Position.y), 1.0));
	output.TexCoord = input.TexCoord + uvOffset;
    return output;
}

#include <box/fragment_uniform.inc.hlsl>


struct Input
{
    float2 TexCoord : INPUT0;
	float2 Corner: INPUT1;
};

float4 main(Input input) : SV_Target0
{
    return lerp(TopColor, BottomColor, input.Corner.y);
}

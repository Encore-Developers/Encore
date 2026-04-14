#include <2d_view.inc.hlsl>
#include <box/vertex_layout.inc.hlsl>
#include <box/vertex_uniform.inc.hlsl>

struct Output
{
    float2 TexCoord : OUTPUT0;
    float2 Corner : OUTPUT1;
    float4 Position : SV_Position;
};


Output main(BoxVertex input)
{
    Output output;
	int CornerIndex = IndexCorner(input.Corner);
	float RealCornerSize = min(CornerSizes[CornerIndex], min(BoxSize.x, BoxSize.y)/2.0);
	float2 RealSize = BoxSize - float2(RealCornerSize, RealCornerSize);
    float2 screenPos = ScaleAboutPoint(BoxPos+input.Position+(RealSize*input.Corner), BoxPos+RealSize*input.Corner, RealCornerSize);
    output.Position = mul(MatrixTransform, float4(screenPos, 0.5, 1.0));
    output.Corner = input.Corner;
	output.TexCoord = input.TexCoord;
    return output;
}

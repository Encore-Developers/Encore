cbuffer UniformBlock : register(b0, space3)
{
    bool showAlpha;
};

Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

struct Input
{
    float2 TexCoord : INPUT0;
};

float4 main(Input input) : SV_Target0
{
    if (!showAlpha) {
        return Texture.Sample(Sampler, input.TexCoord);
    } else {
        float4 color = Texture.Sample(Sampler, input.TexCoord);
        return float4(color.a, color.a, color.a, 1.0);
    }
}

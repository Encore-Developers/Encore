cbuffer GlobalUniforms : register(b0, space1)
{
    float4x4 MatrixTransform;
    uint2 viewportResolution;
};
cbuffer BoxVertexUniform : register(b1, space1)
{
    float2 BoxPos;
    float2 BoxSize;
    float4 CornerSizes;
};

int IndexCorner(float2 CornerValue) {
    if (CornerValue.y < 0.5) {
        if (CornerValue.x < 0.5) {
            return 0;
        } else {
            return 1;
        }
    } else {
        if (CornerValue.x < 0.5) {
            return 2;
        } else {
            return 3;
        }
    }
}
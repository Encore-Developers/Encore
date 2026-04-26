
struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float2 TexCoord : OUTPUT0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
	Output output;
    switch (input.VertexIndex) {
    case 0:
        output.TexCoord = float2(0, 0);
        output.Position = float4(-1, 1, 0, 1);
        break;
    case 1:
        output.TexCoord = float2(0, 1);
        output.Position = float4(-1, -1, 0, 1);
        break;
    case 2:
        output.TexCoord = float2(1, 0);
        output.Position = float4(1, 1, 0, 1);
        break;
    case 3:
        output.TexCoord = float2(1, 1);
        output.Position = float4(1, -1, 0, 1);
        break;
    }
    return output;
}

cbuffer UniformBlock: register(b0, space1) {
	float time;
	float colorMult;
};

struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    float2 pos;
    if (input.VertexIndex == 0)
    {
        pos = (-1.0f).xx;
        output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        if (input.VertexIndex == 1)
        {
            pos = float2(1.0f, -1.0f);
            output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        else
        {
            if (input.VertexIndex == 2)
            {
                pos = float2(0.0f, 1.0f);
				pos.x += sin(time);
                output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
            }
        }
    }
	output.Color *= colorMult;
    output.Position = float4(pos, 0.0f, 1.0f);
    return output;
}

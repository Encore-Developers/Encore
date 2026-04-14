struct BoxVertex
{
    float2 Position : INPUT0;
    float2 TexCoord : INPUT1;
    float2 Corner : INPUT2;
};

float2 ScaleAboutPoint(float2 input, float2 anchor, float scale)
{
    float2 diff = input - anchor;
    return diff*scale + anchor;
}
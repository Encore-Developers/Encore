struct GenericMesh
{
    float3 Position : INPUT0;
    float2 TexCoord : INPUT1;
    uint VertexIndex : SV_VertexID;
};
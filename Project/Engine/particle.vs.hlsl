#include "particle_common.hlsli"

VertexOut VS(VertexIn _in)
{
    VertexOut vout = (VertexOut) 0.f;
    vout.PosW = _in.PosW;
    vout.TexCoord = _in.TexCoord;
    vout.InstID = _in.InstID;
    return vout;
}
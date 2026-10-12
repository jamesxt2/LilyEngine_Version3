#include "billboard_common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    
    return vout;
}
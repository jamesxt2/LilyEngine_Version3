#ifndef _TESS_QUAD_PATCH
#define _TESS_QUAD_PATCH

#include "common.hlsli"


struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float3 PosL : POSITION;
};



struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};


struct HullOut
{
    float3 PosL : POSITION;
};



struct DomainOut
{
    float4 PosH : SV_POSITION;
};

#endif
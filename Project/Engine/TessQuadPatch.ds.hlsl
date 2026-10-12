#include "TessQuadPatch_common.hlsli"

[domain("quad")]
DomainOut DS(PatchTess patchTess,
                float2 uv : SV_DomainLocation,
                const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
    float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
    float3 p = lerp(v1, v2, uv.y);
    
    p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));
    
    float4 posW = mul(float4(p, 1.f), g_World);
    dout.PosH = mul(posW, g_ViewProj);
    
    return dout;
}
#include "TessQuadPatch_common.hlsli"

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch,
                        uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    float3 centerL = 0.25f * (patch[0].PosL + patch[1].PosL + patch[2].PosL + patch[3].PosL);
    float3 centerW = mul(float4(centerL, 1.f), g_World).xyz;
    float d = distance(centerW, g_EyePosW);
    
    const float d0 = 20.f;
    const float d1 = 100.f;
    float tess = 64.f * saturate((d1 - d) / (d1 - d0));
    
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    pt.EdgeTess[3] = tess;
    
    pt.InsideTess[0] = tess;
    pt.InsideTess[1] = tess;
    
    return pt;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.f)]
HullOut HS(InputPatch<VertexOut, 4> p,
            uint i : SV_OutputControlPointID,
            uint patchId : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosL = p[i].PosL;
    return hout;
}
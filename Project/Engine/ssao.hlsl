#include "ssao_common.hlsli"

Texture2D g_RandomVecMap : register(t2);

static const int g_SampleCount = 14;

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    
    vout.TexCoord = g_TexCoords[vid];
    
    vout.PosH = float4(2.f * vout.TexCoord.x - 1.f, 1.f - 2.f * vout.TexCoord.y, 0.f, 1.f);
    
    float4 ph = mul(vout.PosH, g_InvProj);
    vout.PosV = ph.xyz / ph.w;
    
    return vout;
}

float OcclusionFunction(float distZ)
{
    float occlusion = 0.f;
    if (distZ > g_SurfaceEpsilon)
    {
        float fadeLength = g_OcclusionFadeEnd - g_OcclusionFadeStart;
        occlusion = saturate((g_OcclusionFadeEnd - distZ) / fadeLength);
    }
    return occlusion;
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 n = g_NormalMap.SampleLevel(g_SamPointClamp, pin.TexCoord, 0.f).xyz;
    float pz = g_DepthMap.SampleLevel(g_SamDepthMap, pin.TexCoord, 0.f).r;
    pz = NdcDepthToViewDepth(pz);
    
    float3 p = (pz / pin.PosV.z) * pin.PosV;
    
    float3 randVec = 2.f * g_RandomVecMap.SampleLevel(g_SamLinearWrap, 
                                            4.f * pin.TexCoord, 0.f).rgb - 1.f;

    float occlusionSum = 0.f;
    for (int i = 0; i < g_SampleCount; ++i)
    {
        float3 offset = reflect(g_OffsetVectors[i].xyz, randVec);
        float flip = sign(dot(offset, n));
        
        float3 q = p + flip * g_OcclusionRadius * offset;
        
        float4 projQ = mul(float4(q, 1.f), g_ProjTex);
        projQ /= projQ.w;
        
        float rz = g_DepthMap.SampleLevel(g_SamDepthMap, projQ.xy, 0.f).r;
        rz = NdcDepthToViewDepth(rz);
        
        float3 r = (rz / q.z) * q;
        
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.f);
        float occlusion = dp * OcclusionFunction(distZ);
        
        occlusionSum += occlusion;
    }

    occlusionSum /= g_SampleCount;
    float access = 1.f - occlusionSum;
    return saturate(pow(access, 2.f));
}

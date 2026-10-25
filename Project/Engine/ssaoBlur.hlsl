#include "ssao_common.hlsli"

Texture2D g_InputMap : register(t2);

static const int g_BlurRadius = 5;

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    
    vout.TexCoord = g_TexCoords[vid];
    vout.PosH = float4(2.f * vout.TexCoord.x - 1.f, 1.f - 2.f * vout.TexCoord.y, 0.f, 1.f);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float blurWeights[12] =
    {
        g_BlurWeights[0].x, g_BlurWeights[0].y,
        g_BlurWeights[0].z, g_BlurWeights[0].w,
        g_BlurWeights[1].x, g_BlurWeights[1].y,
        g_BlurWeights[1].z, g_BlurWeights[1].w,
        g_BlurWeights[2].x, g_BlurWeights[2].y,
        g_BlurWeights[2].z, g_BlurWeights[2].w
    };

    float2 texOffset;
    if (g_HorizontalBlur)
        texOffset = float2(g_InvRenderTargetSize.x, 0.f);
    else
        texOffset = float2(0.f, g_InvRenderTargetSize.y);
    
    float4 color = blurWeights[g_BlurRadius] * g_InputMap.SampleLevel(g_SamPointClamp, pin.TexCoord, 0.f);
    float totalWeight = blurWeights[g_BlurRadius];
    
    float3 centerNormal = g_NormalMap.SampleLevel(g_SamPointClamp, pin.TexCoord, 0.f).xyz;
    float centerDepth = NdcDepthToViewDepth(g_DepthMap.SampleLevel(g_SamDepthMap, pin.TexCoord, 0.f).r);
    
    for (float i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        if (i == 0)
            continue;
        
        float2 tex = pin.TexCoord + i * texOffset;
        
        float3 neighbourNormal = g_NormalMap.SampleLevel(g_SamPointClamp, tex, 0.f).xyz;
        float neighbourDepth = NdcDepthToViewDepth(g_DepthMap.SampleLevel(g_SamDepthMap, tex, 0.f).r);
        
        if (dot(neighbourNormal, centerNormal) >= 0.8f && abs(neighbourDepth - centerDepth) <= 0.2f)
        {
            float weight = blurWeights[i + g_BlurRadius];
            color += weight * g_InputMap.SampleLevel(g_SamPointClamp, tex, 0.f);
            totalWeight += weight;
        }
    }

    return color / totalWeight;
}
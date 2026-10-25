#ifndef _FXAA
#define _FXAA

Texture2D g_SceneTexture : register(t0);
SamplerState g_SamLinearClamp : register(s1);

cbuffer SCREEN_DATA : register(b0)
{
    float2 g_InvScreenSize;
}

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    vout.TexCoord = float2((vid << 1) & 2, vid & 2);
    vout.PosH = float4(vout.TexCoord * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);
    return vout;
}

float Luma(float3 rgb)
{
    return dot(rgb, float3(0.299f, 0.587f, 0.114f));
}

float4 PS(VertexOut pin) : SV_Target
{
    float2 uv = pin.TexCoord;
    float2 texelSize = g_InvScreenSize;
    
    float lumaM = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv, 0.f).rgb);
    float lumaN = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(0.f, texelSize.y), 0.f).rgb);
    float lumaS = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv - float2(0.f, texelSize.y), 0.f).rgb);
    float lumaE = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(texelSize.x, 0.f), 0.f).rgb);
    float lumaW = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv - float2(texelSize.x, 0.f), 0.f).rgb);
    
    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaE, lumaW)));
    float lumaRange = lumaMax - lumaMin;
    
    if (lumaRange < max(0.0312f, lumaMax * 0.125f))
        return g_SceneTexture.SampleLevel(g_SamLinearClamp, uv, 0.f);
    
    float lumaNW = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(-texelSize.x, -texelSize.y), 0.f).rgb);
    float lumaNE = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(texelSize.x, -texelSize.y), 0.f).rgb);
    float lumaSW = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(-texelSize.x, texelSize.y), 0.f).rgb);
    float lumaSE = Luma(g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + float2(texelSize.x, texelSize.y), 0.f).rgb);
    
    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25f * 0.125f), 1.f / 128.f);
    float rcpDirMin = 1.f / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(float2(8.f, 8.f), max(float2(-8.f, -8.f), dir * rcpDirMin)) * texelSize;
    
    float3 rgbA = 0.5f * (
        g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + dir * (1.f / 3.f - 0.5f), 0.f).rgb +
        g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + dir * (2.f / 3.f - 0.5f), 0.f).rgb
    );
    float3 rgbB = rgbA * 0.5f + 0.25f * (
        g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + dir * (-0.5f), 0.f).rgb +
        g_SceneTexture.SampleLevel(g_SamLinearClamp, uv + dir *  0.5f, 0.f).rgb
    );
    
    float lumaB = Luma(rgbB);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        return float4(rgbA, 1.f);
    return float4(rgbB, 1.f);
}

#endif
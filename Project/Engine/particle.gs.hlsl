#include "particle_common.hlsli"

[maxvertexcount(8)]
void GS(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    if (g_Particle[gin[0].InstID].IsActive == 0)
        return;
    
    float4 ViewPos = mul(mul(float4(gin[0].PosW + g_Particle[gin[0].InstID].RelativePosition, 1.f), g_World), g_View);
    GeoOut output[4] =
    {
        (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f
    };
    GeoOut output_cross[4] =
    {
        (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f
    };
    
    // View Space
    // 0---1
    // | \ |
    // 3---2
    output[0].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 1.f);
    output[1].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 1.f);
    output[2].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 1.f);
    output[3].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 1.f);

    output[0].TexCoord = float2(0.f, 0.f);
    output[1].TexCoord = float2(1.f, 0.f);
    output[2].TexCoord = float2(1.f, 1.f);
    output[3].TexCoord = float2(0.f, 1.f);

    if (g_Particle[gin[0].InstID].VelocityAlignment)
    {
        float3 vR = normalize(mul(float4(g_Particle[gin[0].InstID].Velocity, 0.f), g_View).xyz);
        float3 vF = normalize(cross(vR, float3(0.f, 1.f, 0.f)));
        float3 vU = normalize(cross(vF, vR));
        
        float3x3 vRot = { vR, vU, vF };
        
        output_cross[0].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
            0.f, g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f);
        output_cross[1].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
            0.f, g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f);
        output_cross[2].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
            0.f, -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f);
        output_cross[3].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
            0.f, -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f);
        
        output_cross[0].TexCoord = float2(0.f, 0.f);
        output_cross[1].TexCoord = float2(1.f, 0.f);
        output_cross[2].TexCoord = float2(1.f, 1.f);
        output_cross[3].TexCoord = float2(0.f, 1.f);
        
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            output[i].PosH.xyz = mul(output[i].PosH.xyz, vRot);
            output_cross[i].PosH.xyz = mul(output_cross[i].PosH.xyz, vRot);
        }
    }

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        output[i].PosH.xyz += ViewPos.xyz;
        output[i].PosH.w = ViewPos.w;
        output[i].PosH = mul(output[i].PosH, g_Proj);
        output[i].InstID = gin[0].InstID;
    }
    
    triStream.Append(output[0]);
    triStream.Append(output[1]);
    triStream.Append(output[3]);
    triStream.Append(output[2]);
    triStream.RestartStrip();
    
    if (g_Particle[gin[0].InstID].VelocityAlignment)
    {
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            output_cross[i].PosH.xyz += ViewPos.xyz;
            output_cross[i].PosH.w = ViewPos.w;
            output_cross[i].PosH = mul(output_cross[i].PosH, g_Proj);
            output_cross[i].InstID = gin[0].InstID;
        }
        triStream.Append(output_cross[0]);
        triStream.Append(output_cross[1]);
        triStream.Append(output_cross[3]);
        triStream.Append(output_cross[2]);
        triStream.RestartStrip();
    }
}

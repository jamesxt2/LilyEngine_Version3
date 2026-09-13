cbuffer TRANSFORM : register(b0)
{
    row_major matrix g_matModel;
    row_major matrix g_matView;
    row_major matrix g_matProj;
    
    row_major matrix g_matMV;
    row_major matrix g_matMVP;
}

struct VertexIn
{
    float3 vPos : POSITION;
    float4 vColor : COLOR;
    float2 vUV : TEXCOORD;
};

struct VertexOut
{
    float4 vPosition : SV_Position;
    float4 vColor : COLOR;
    float2 vUV : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
	// Transform to homogeneous clip space.
    vout.vPosition = mul(float4(vin.vPos, 1.0f), g_matMVP);
	
	// Just pass vertex color into the pixel shader.
    vout.vColor = vin.vColor;
    
    vout.vUV = vin.vUV;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.vColor;
}


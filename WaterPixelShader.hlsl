

struct PixelShaderInput
{
    float4 pos : SV_Position;
    float3 normal : NORMAL0;
    float4 tangent : TANGENT0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
    float4 reflectPosition : TEXCOORD1;
    float4 refractPosition : TEXCOORD2;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = input.color;

    color.a = 0.5f;
    return color;
}
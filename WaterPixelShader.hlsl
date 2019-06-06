
SamplerState SampleType;
Texture2D refractionTexture : register(t0);
Texture2D reflectionTexture : register(t1);
Texture2D normalTexture : register(t2);

struct PixelShaderInput
{
    float4 pos : SV_Position;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 color : COLOR;
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
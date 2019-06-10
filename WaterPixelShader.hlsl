
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
    float2 reflectTexCoord;
    float2 refractTexCoord;
    
    // Calculate the projected reflection texture coordinates.
    reflectTexCoord.x = input.reflectPosition.x / input.reflectPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectPosition.y / input.reflectPosition.w / 2.0f + 0.5f;

    // Calculate the projected refraction texture coordinates.
    refractTexCoord.x = input.refractPosition.x / input.refractPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractPosition.y / input.refractPosition.w / 2.0f + 0.5f;

    float4 reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    float4 refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    float4 color = lerp(reflectionColor, refractionColor, 0.6f) * input.color;

    return color;
}
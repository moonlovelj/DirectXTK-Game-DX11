////////////////////////////////////////////////////////////////////////////////
// Filename: reflection.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TEXTURES //
//////////////
Texture2D gTexture : register(t0);

//////////////
// SAMPLERS //
//////////////
SamplerState SampleType;

//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer LightBuffer
{
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
    float3 lightDirection;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
    // Invert the light direction for calculations.
    float3 lightDir = -lightDirection;

    // Sample the color texture.
    float4 textureColor = gTexture.Sample(SampleType, input.tex);

    float4 color = lightAmbientColor;

    float lightIntensity = saturate(dot(input.normal, lightDir));

    color += lightIntensity * lightDiffuseColor;

    color = saturate(color);

    color = saturate(color * input.color * textureColor);

    return color;
}
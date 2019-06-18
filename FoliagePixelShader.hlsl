
//////////////
// TEXTURES //
//////////////
Texture2D gTexture : register(t0);

//////////////
// SAMPLERS //
//////////////
SamplerState SampleType;



//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
    float4 color = gTexture.Sample(SampleType, input.tex);
    clip(color.a - 0.25f);
    return color;
}
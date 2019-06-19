
Texture2D foliageTexture : register(t0);

SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color = foliageTexture.Sample(SampleType, input.tex);
    //clip(color.a - 0.25f);
//color.r = 1.f;
//color.g = 0.f;
//color.b = 0.f;
//color.a = 1.f;
    return color;
}
Texture2D<float4> CloudTexture : register(t0);
Texture2D<float4> PerturbTexture : register(t1);
sampler TextureSampler : register(s0);

cbuffer CLOUD_PARAMETERS : register(b0)
{
    float translation;
    float scale;
    float brightness;
}

float4 main(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    // Translate the texture coordinate sampling location by the translation value.
    texCoord.xy = texCoord.xy + translation;

    // Sample the texture value from the perturb texture using the translated texture coordinates.
    float4 perturbValue = PerturbTexture.Sample(TextureSampler, texCoord);

    // Multiply the perturb value by the perturb scale.
    perturbValue = perturbValue * scale;

    // Add the texture coordinates as well as the translation value to get the perturbed texture coordinate sampling location.
    perturbValue.xy = perturbValue.xy + texCoord.xy + translation;

    // Now sample the color from the cloud texture using the perturbed sampling coordinates.
    float4 cloudColor = CloudTexture.Sample(TextureSampler, perturbValue.xy);

    // Reduce the color cloud by the brightness value.
    cloudColor = cloudColor * brightness;

    return cloudColor;
}
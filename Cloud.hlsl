Texture2D<float4> CloudTexture0 : register(t0);
Texture2D<float4> CloudTexture1 : register(t1);
sampler TextureSampler : register(s0);

cbuffer CLOUD_PARAMETERS : register(b0)
{
    float firstTranslationX;
    float firstTranslationZ;
    float secondTranslationX;
    float secondTranslationZ;
    float brightness;
}

float4 main(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float2 sampleLocation;
    float4 textureColor1;
    float4 textureColor2;
    float4 finalColor;

    // Translate the position where we sample the pixel from using the first texture translation values.
    sampleLocation.x = texCoord.x + firstTranslationX;
    sampleLocation.y = texCoord.y + firstTranslationZ;

    // Sample the pixel color from the first cloud texture using the sampler at this texture coordinate location.
    textureColor1 = CloudTexture0.Sample(TextureSampler, sampleLocation);

    // Translate the position where we sample the pixel from using the second texture translation values.
    sampleLocation.x = texCoord.x + secondTranslationX;
    sampleLocation.y = texCoord.y + secondTranslationZ;

    // Sample the pixel color from the second cloud texture using the sampler at this texture coordinate location.
    textureColor2 = CloudTexture1.Sample(TextureSampler, sampleLocation);

    // Combine the two cloud textures evenly.
    finalColor = lerp(textureColor1, textureColor2, 0.5f);

    // Reduce brightness of the combined cloud textures by the input brightness value.
    finalColor = finalColor * brightness;

    return finalColor;
}

SamplerState SampleType;
Texture2D refractionTexture : register(t0);
Texture2D reflectionTexture : register(t1);
Texture2D normalTexture : register(t2);

cbuffer WaterBuffer: register (b0)
{
    float3 lightDirection;
    float waterTranslation;
    float reflectRefractScale;
    float specularShininess;
};

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
    float3 viewDirection : TEXCOORD3;
    float2 tex1 : TEXCOORD4;
    float2 tex2 : TEXCOORD5;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
 // Move the position the water normal is sampled from to simulate moving water.	
input.tex1.y += waterTranslation;
input.tex2.y += waterTranslation;

// Sample the normal from the normal map texture using the two different tiled and translated coordinates.
float4 normalMap1 = normalTexture.Sample(SampleType, input.tex1);
float4 normalMap2 = normalTexture.Sample(SampleType, input.tex2);

// Expand the range of the normal from (0,1) to (-1,+1).
float3 normal1 = (normalMap1.rgb * 2.0f) - 1.0f;
float3 normal2 = (normalMap2.rgb * 2.0f) - 1.0f;

// Combine the normals to add the normal maps together.
float3 normal = normalize(normal1 + normal2);

float2 reflectTexCoord;
float2 refractTexCoord;

// Calculate the projected reflection texture coordinates.
reflectTexCoord.x = input.reflectPosition.x / input.reflectPosition.w / 2.0f + 0.5f;
reflectTexCoord.y = -input.reflectPosition.y / input.reflectPosition.w / 2.0f + 0.5f;

// Calculate the projected refraction texture coordinates.
refractTexCoord.x = input.refractPosition.x / input.refractPosition.w / 2.0f + 0.5f;
refractTexCoord.y = -input.refractPosition.y / input.refractPosition.w / 2.0f + 0.5f;

reflectTexCoord += normal.xy * reflectRefractScale;
refractTexCoord += normal.xy * reflectRefractScale;

float4 reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
float4 refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

// Get a modified viewing direction of the camera that only takes into account height.
float3 heightView;
heightView.x = input.viewDirection.y;
heightView.y = input.viewDirection.y;
heightView.z = input.viewDirection.y;

// Now calculate the fresnel term based solely on height.
float r = (1.2f - 1.0f) / (1.2f + 1.0f);
float fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normal, heightView), 2)));

float4 color = saturate(lerp(saturate(reflectionColor), saturate(refractionColor * input.color) , fresnelFactor));

// Calculate the reflection vector using the normal and the direction of the light.
float3 reflection = -reflect(normalize(lightDirection), normal);

// Calculate the specular light based on the reflection and the camera position.
float specular = dot(normalize(reflection), normalize(input.viewDirection));

// Check to make sure the specular was positive so we aren't adding black spots to the water.
if (specular > 0.0f)
{
    // Increase the specular light by the shininess value.
    specular = pow(specular, specularShininess);

    // Add the specular to the final color.
    color = saturate(color + specular);
}

return color;
}
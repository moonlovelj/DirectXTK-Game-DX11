
cbuffer MatrixBuffer : register (b0)
{
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
};

cbuffer ReflectionBuffer
{
    row_major matrix reflectionMatrix;
};

cbuffer CamNormBuffer
{
    float3 cameraPosition;
    float normalMapTilingX;
    float normalMapTilingY;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float4 pos : SV_Position;
    float3 normal : NORMAL0;
    float4 tangent : TANGENT0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

// Per-pixel color data passed through the pixel shader.
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

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.pos.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.pos = mul(input.pos, worldMatrix);
    output.pos = mul(output.pos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);

    output.reflectPosition = mul(input.pos, worldMatrix);
    output.reflectPosition = mul(output.reflectPosition, reflectionMatrix);
    output.reflectPosition = mul(output.reflectPosition, projectionMatrix);

    output.refractPosition = mul(input.pos, worldMatrix);
    output.refractPosition = mul(output.refractPosition, viewMatrix);
    output.refractPosition = mul(output.refractPosition, projectionMatrix);

    output.color = input.color;

    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only and then normalize the final value.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    // Calculate the tangent vector against the world matrix only and then normalize the final value.
    output.tangent = mul(input.tangent.xyz, (float3x3)worldMatrix);
    output.tangent = normalize(output.tangent);

    output.binormal = cross(output.normal, output.tangent);
    output.binormal = normalize(output.binormal);
    output.binormal.xyz = output.binormal.xyz * input.tangent.w;

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    output.viewDirection = cameraPosition.xyz - mul(input.pos, worldMatrix).xyz;

    // Normalize the viewing direction vector.
    output.viewDirection = normalize(output.viewDirection);

    output.tex1 = input.tex / normalMapTilingX;
    output.tex2 = input.tex / normalMapTilingY;

    return output;
}

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

    output.color = input.color;

    output.tex = input.tex;

    return output;
}
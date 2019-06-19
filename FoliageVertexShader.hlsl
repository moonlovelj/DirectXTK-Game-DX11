
cbuffer MatrixBuffer
{
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 positionOffset: TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
};

VS_OUTPUT main(VertexInputType input)
{
    VS_OUTPUT output;

    input.position.w = 1.0f;
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position.xyz += input.positionOffset;
    output.worldPosition = output.position.xyz;

    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    return output;
}
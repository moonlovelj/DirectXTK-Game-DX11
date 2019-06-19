cbuffer FoliageBuffer
{
    float3 cameraPosition;
    float  treeBillWidth;
    float  treeBillHeight;
}

cbuffer MatrixBuffer
{
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<PixelInputType> OutputStream)
{
    float halfWidth = treeBillWidth / 2.0f;

    float3 planeNormal = input[0].worldPosition - cameraPosition;
    planeNormal.y = 0.0f;
    planeNormal = normalize(planeNormal);

    float3 upVector = float3(0.0f, 1.0f, 0.0f);

    float3 rightVector = normalize(cross(planeNormal, upVector)); 

    rightVector = rightVector * halfWidth;

    upVector = float3(0, treeBillHeight, 0);

    float3 vert[4];
    vert[0] = input[0].worldPosition - rightVector; // Get bottom left vertex
    vert[1] = input[0].worldPosition + rightVector; // Get bottom right vertex
    vert[2] = input[0].worldPosition - rightVector + upVector; // Get top left vertex
    vert[3] = input[0].worldPosition + rightVector + upVector; // Get top right vertex
    
    
    // Get billboards texture coordinates
    float2 texCoord[4];
    texCoord[0] = float2(0, 0);
    texCoord[1] = float2(1, 0);
    texCoord[2] = float2(0, 1);
    texCoord[3] = float2(1, 1);

    for (int i = 0; i < 4; i++)
    {
        PixelInputType output;
        output.position = mul(float4(vert[i], 1.0f), viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        output.tex = texCoord[i];
        OutputStream.Append(output);
    }
}
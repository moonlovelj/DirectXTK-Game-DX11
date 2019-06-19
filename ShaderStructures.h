#pragma once

#include "pch.h"

struct MatrixBufferData
{
    DirectX::SimpleMath::Matrix worldMatrix;
    DirectX::SimpleMath::Matrix viewMatrix;
    DirectX::SimpleMath::Matrix projectMatrix;
};

struct ReflectionLightBufferData
{
    DirectX::SimpleMath::Vector4 lightAmbientColor;
    DirectX::SimpleMath::Vector4 lightDiffuseColor;
    DirectX::SimpleMath::Vector3 lightDirection;
    uint8_t padding[4];
};

struct ClipPlaneConstBufferData
{
    DirectX::SimpleMath::Plane clipPlane;
};

struct ReflectionBufferData
{
    DirectX::SimpleMath::Matrix reflectionMatrix;
};

struct WaterBufferData
{
    DirectX::SimpleMath::Vector3 lightDirection;
    float waterTranslation;
    float reflectRefractScale;
    float specularShininess;
    uint8_t padding[8];
};

struct CamNormBufferData
{
    DirectX::SimpleMath::Vector3 cameraPosition;
    float normalMapTilingX;
    float normalMapTilingY;
    uint8_t padding[12];
};

struct FoliageBufferData
{
    DirectX::SimpleMath::Vector3 cameraPosition;
    float  treeBillWidth;
    float  treeBillHeight;
    uint8_t padding[12];
};
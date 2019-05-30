#pragma once
#include "pch.h"

struct ReflectionMatrixBufferData
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
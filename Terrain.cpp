#include "pch.h"
#include "Terrain.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Terrain::Terrain(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext)
{
    m_terrainWidth = 100;
    m_terrainHeight = 100;

    m_states = std::make_unique<CommonStates>(device);

    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    DX::ThrowIfFailed(
        device->CreateInputLayout(VertexPositionColor::InputElements,
            VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_inputLayout.ReleaseAndGetAddressOf()));

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(deviceContext);
}


Terrain::~Terrain()
{
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
}

void Terrain::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
    m_effect->SetWorld(world);
    m_effect->SetView(view);
    m_effect->SetProjection(proj);
    m_effect->Apply(deviceContext);
    deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_batch->Begin();
    for (int i = 0; i < m_terrainWidth; i++)
    {
        VertexPositionColor v1(Vector3(static_cast<float>(i), 0.f, 0.f), Colors::White);
        VertexPositionColor v2(Vector3(static_cast<float>(i), 0.f, m_terrainHeight - 1.f), Colors::White);
        m_batch->DrawLine(v1, v2);
    }
    for (int i = 0; i < m_terrainHeight; i++)
    {
        VertexPositionColor v1(Vector3(0.f, 0.f, static_cast<float>(i)), Colors::White);
        VertexPositionColor v2(Vector3(m_terrainWidth - 1.f, 0.f, static_cast<float>(i)), Colors::White);
        m_batch->DrawLine(v1, v2);
    }
    m_batch->End();
}
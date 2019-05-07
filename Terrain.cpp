#include "pch.h"
#include "Terrain.h"
#include "FastNoise.h"

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

    FastNoise myNoise;
    myNoise.SetNoiseType(FastNoise::PerlinFractal);
    for (int i = 0; i < m_terrainWidth - 1; i++)
    {
        for (int j = 0; j < m_terrainHeight - 1; j++)
        {
            GeometricPrimitive::VertexType vertex0, vertex1, vertex2, vertex3;
            vertex0.position.x = static_cast<float>(i);
            vertex0.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i), static_cast<FN_DECIMAL>(j));
            vertex0.position.z = static_cast<float>(j);

            vertex1.position.x = static_cast<float>(i + 1);
            vertex1.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i + 1), static_cast<FN_DECIMAL>(j));
            vertex1.position.z = static_cast<float>(j);

            vertex2.position.x = static_cast<float>(i + 1);
            vertex2.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i + 1), static_cast<FN_DECIMAL>(j + 1));
            vertex2.position.z = static_cast<float>(j + 1);

            vertex3.position.x = static_cast<float>(i);
            vertex3.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i), static_cast<FN_DECIMAL>(j + 1));
            vertex3.position.z = static_cast<float>(j + 1);

            auto baseIndex = m_vertices.size();
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 0));
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 1));
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 2));
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 0));
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 2));
            m_indices.emplace_back(static_cast<uint16_t>(baseIndex + 3));

            m_vertices.emplace_back(vertex0);
            m_vertices.emplace_back(vertex1);
            m_vertices.emplace_back(vertex2);
            m_vertices.emplace_back(vertex3);
        }
    }

    m_shape = GeometricPrimitive::CreateCustom(deviceContext, m_vertices, m_indices);
}


Terrain::~Terrain()
{
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_shape.reset();
}

void Terrain::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
    /*m_effect->SetWorld(world);
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
    m_batch->End();*/
    deviceContext;
    m_shape->Draw(world, view, proj, Colors::White, nullptr, false);
}
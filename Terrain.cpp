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
    m_effect->EnableDefaultLighting();
    m_effect->SetSpecularColor({ 0.f, 0.f, 0.f, 0.f });
    //m_effect->SetLightEnabled(0, false);
    //m_effect->SetLightEnabled(1, false);
    //m_effect->SetAmbientLightColor({ 0.5f, 0.5f, 0.5f, 1.0f });
   /* m_effect->SetSpecularColor({0.f, 0.f, 0.f, 0.f});
    m_effect->SetLightingEnabled(true);
    m_effect->SetLightEnabled(0, true);
    m_effect->SetAmbientLightColor({ 0.05f, 0.05f, 0.05f, 0.f });
    m_effect->SetLightDiffuseColor(0, { 1.0000000f, 0.9607844f, 0.8078432f, 0 });
    m_effect->SetLightDirection(0, { 0.0f, 0.0f, 0.75f });*/

    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    DX::ThrowIfFailed(
        device->CreateInputLayout(VertexPositionNormalTexture::InputElements,
            VertexPositionNormalTexture::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_inputLayout.ReleaseAndGetAddressOf()));

    //m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(deviceContext);

    FastNoise myNoise;
    myNoise.SetNoiseType(FastNoise::PerlinFractal);
    m_vertices.resize(m_terrainWidth * m_terrainHeight);
    for (int i = 0; i < m_terrainWidth - 1; i++)
    {
        for (int j = 0; j < m_terrainHeight - 1; j++)
        {
            auto index0 = j * m_terrainWidth + i;
            auto index1 = j * m_terrainWidth + i + 1;
            auto index2 = (j + 1) * m_terrainWidth + i + 1;
            auto index3 = (j + 1) * m_terrainWidth + i;
            auto& vertex0 = m_vertices[index0];
            auto& vertex1 = m_vertices[index1];
            auto& vertex2 = m_vertices[index2];
            auto& vertex3 = m_vertices[index3];

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

            m_indices.emplace_back(static_cast<uint16_t>(index0));
            m_indices.emplace_back(static_cast<uint16_t>(index1));
            m_indices.emplace_back(static_cast<uint16_t>(index2));
            m_indices.emplace_back(static_cast<uint16_t>(index0));
            m_indices.emplace_back(static_cast<uint16_t>(index2));
            m_indices.emplace_back(static_cast<uint16_t>(index3));
        }
    }

    std::vector<Vector3> normals(m_terrainWidth * m_terrainHeight);
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        auto vertex0 = Vector3(m_vertices[m_indices[i + 0]].position);
        auto vertex1 = Vector3(m_vertices[m_indices[i + 1]].position);
        auto vertex2 = Vector3(m_vertices[m_indices[i + 2]].position);
        auto normal = (vertex0 - vertex1).Cross(vertex2 - vertex1);
        normals[m_indices[i + 0]] += normal;
        normals[m_indices[i + 1]] += normal;
        normals[m_indices[i + 2]] += normal;
    }

    for (auto& normal : normals)
    {
        normal.Normalize();
    }

    for (size_t i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i].normal.x = normals[i].x;
        m_vertices[i].normal.y = normals[i].y;
        m_vertices[i].normal.z = normals[i].z;
    }

    m_shape = GeometricPrimitive::CreateCustom(deviceContext, m_vertices, m_indices);
}


Terrain::~Terrain()
{
    m_states.reset();
    m_effect.reset();
    //m_batch.reset();
    m_inputLayout.Reset();
    m_shape.reset();
}

void Terrain::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
    m_effect->SetWorld(world);
    m_effect->SetView(view);
    m_effect->SetProjection(proj);
    m_effect->Apply(deviceContext);
    /*deviceContext->IASetInputLayout(m_inputLayout.Get());
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
    m_shape->Draw(m_effect.get(), m_inputLayout.Get());
}
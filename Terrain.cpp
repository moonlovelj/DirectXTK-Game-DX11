#include "pch.h"
#include "Terrain.h"
//#include "FastNoise.h"
#include "Common.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

// Helper for creating a D3D vertex or index buffer.
template<typename T>
void CreateBuffer(_In_ ID3D11Device* device, T const& data, D3D11_BIND_FLAG bindFlags, _Outptr_ ID3D11Buffer** pBuffer)
{
    assert(pBuffer != nullptr);

    uint64_t sizeInBytes = uint64_t(data.size()) * sizeof(typename T::value_type);

    if (sizeInBytes > uint64_t(D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM * 1024u * 1024u))
        throw std::exception("Buffer too large for DirectX 11");

    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = static_cast<UINT>(sizeInBytes);
    bufferDesc.BindFlags = bindFlags;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA dataDesc = {};

    dataDesc.pSysMem = data.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&bufferDesc, &dataDesc, pBuffer)
    );

    _Analysis_assume_(*pBuffer != 0);

    SetDebugObjectName(*pBuffer, "DirectXTK:GeometricPrimitive");
}

Terrain::Terrain(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext)
{
    deviceContext;

    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"terrain.dds", resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf()));

    m_states = std::make_unique<CommonStates>(device);

    m_reflectionLightBufferData.lightAmbientColor = { 0.05f, 0.05f, 0.05f, 1.f };
    m_reflectionLightBufferData.lightDiffuseColor = { 1.f, 1.f, 1.f, 1.f };
    m_reflectionLightBufferData.lightDirection = { 0.f, -1.f, 2.f };

    m_clipPlaneConstBufferData.clipPlane.x = 0.f;
    m_clipPlaneConstBufferData.clipPlane.y = 1.f;
    m_clipPlaneConstBufferData.clipPlane.z = 0.f;
    m_clipPlaneConstBufferData.clipPlane.w = 100.f;

    CD3D11_BUFFER_DESC constDesc(sizeof(MatrixBufferData), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_reflectionMatrixBuffer)
    );

    constDesc.ByteWidth = sizeof(ReflectionLightBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_reflectionLightBuffer)
    );

    constDesc.ByteWidth = sizeof(ClipPlaneConstBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_clipPlaneConstBuffer)
    );

    deviceContext->UpdateSubresource(m_reflectionLightBuffer.Get(), 0, nullptr, &m_reflectionLightBufferData, 0, 0);

    deviceContext->UpdateSubresource(m_clipPlaneConstBuffer.Get(), 0, nullptr, &m_clipPlaneConstBufferData, 0, 0);

    auto blobVertex = DX::ReadData(L"ReflectionVertexShader.cso");
    DX::ThrowIfFailed(device->CreateVertexShader(blobVertex.data(), blobVertex.size(),
        nullptr, m_terrainVertexShader.ReleaseAndGetAddressOf()));

    auto blobPixel = DX::ReadData(L"ReflectionPixelShader.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(blobPixel.data(), blobPixel.size(),
        nullptr, m_terrainPixelShader.ReleaseAndGetAddressOf()));

    DX::ThrowIfFailed(
        device->CreateInputLayout(VertexType::InputElements,
            VertexType::InputElementCount,
            blobVertex.data(), blobVertex.size(),
            m_inputLayout.ReleaseAndGetAddressOf()));

    /*FastNoise myNoise;
    myNoise.SetNoiseType(FastNoise::PerlinFractal);*/
    std::vector<uint8_t> heightMap;
    int heightMapWidth, heightMapHeight;
    ReadBitmap("heightmap01.bmp", heightMap, heightMapWidth, heightMapHeight);
    m_terrainWidth = heightMapWidth;
    m_terrainHeight = heightMapHeight;

    std::vector<uint8_t> colorMap;
    int colorMapWidth, colorMapHeight;
    ReadBitmap("colorm01.bmp", colorMap, colorMapWidth, colorMapHeight);
    if (colorMapWidth != m_terrainWidth || colorMapHeight != m_terrainHeight)
        throw std::exception("Size of color map is not same as size of height map");

    m_vertices.resize(m_terrainWidth * m_terrainHeight);
    auto textureGrids = std::max(m_terrainWidth / m_terrainTextureRepeat, 1);
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
            auto tu = (i % textureGrids) * (1.f / textureGrids) + i / textureGrids;
            auto tv = (j % textureGrids) * (1.f / textureGrids) + j / textureGrids;
            auto u = (i % textureGrids + 1) * (1.f / textureGrids) + i / textureGrids;
            auto v = (j % textureGrids + 1) * (1.f / textureGrids) + j / textureGrids;

            vertex0.position.x = static_cast<float>(i);
            //vertex0.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i), static_cast<FN_DECIMAL>(j));
            vertex0.position.y = m_terrainMaxHeight * static_cast<int>(heightMap[(j * heightMapWidth + i) * 3]) / 255.f;
            vertex0.position.z = static_cast<float>(j);
            vertex0.textureCoordinate.x = tu;
            vertex0.textureCoordinate.y = tv;
            vertex0.color.x = static_cast<int>(colorMap[(j * colorMapWidth + i) * 3] + 2) / 255.f * 2.f;
            vertex0.color.y = static_cast<int>(colorMap[(j * colorMapWidth + i) * 3] + 1) / 255.f * 2.f;
            vertex0.color.z = static_cast<int>(colorMap[(j * colorMapWidth + i) * 3] + 0) / 255.f * 2.f;
            vertex0.color.w = 1.f;

            vertex1.position.x = static_cast<float>(i + 1);
            //vertex1.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i + 1), static_cast<FN_DECIMAL>(j));
            vertex1.position.y = m_terrainMaxHeight * static_cast<int>(heightMap[(j * heightMapWidth + i + 1) * 3]) / 255.f;
            vertex1.position.z = static_cast<float>(j);
            vertex1.textureCoordinate.x = u;
            vertex1.textureCoordinate.y = tv;
            vertex1.color.x = static_cast<int>(colorMap[(j * colorMapWidth + i + 1) * 3] + 2) / 255.f * 2.f;
            vertex1.color.y = static_cast<int>(colorMap[(j * colorMapWidth + i + 1) * 3] + 1) / 255.f * 2.f;
            vertex1.color.z = static_cast<int>(colorMap[(j * colorMapWidth + i + 1) * 3] + 0) / 255.f * 2.f;
            vertex1.color.w = 1.f;

            vertex2.position.x = static_cast<float>(i + 1);
            //vertex2.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i + 1), static_cast<FN_DECIMAL>(j + 1));
            vertex2.position.y = m_terrainMaxHeight * static_cast<int>(heightMap[((j + 1) * heightMapWidth + i + 1) * 3]) / 255.f;
            vertex2.position.z = static_cast<float>(j + 1);
            vertex2.textureCoordinate.x = u;
            vertex2.textureCoordinate.y = v;
            vertex2.color.x = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i + 1) * 3] + 2) / 255.f * 2.f;
            vertex2.color.y = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i + 1) * 3] + 1) / 255.f * 2.f;
            vertex2.color.z = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i + 1) * 3] + 0) / 255.f * 2.f;
            vertex2.color.w = 1.f;

            vertex3.position.x = static_cast<float>(i);
            //vertex3.position.y = m_terrainMaxHeight * myNoise.GetNoise(static_cast<FN_DECIMAL>(i), static_cast<FN_DECIMAL>(j + 1));
            vertex3.position.y = m_terrainMaxHeight * static_cast<int>(heightMap[((j + 1) * heightMapWidth + i) * 3]) / 255.f;
            vertex3.position.z = static_cast<float>(j + 1);
            vertex3.textureCoordinate.x = tu;
            vertex3.textureCoordinate.y = v;
            vertex3.color.x = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i) * 3] + 2) / 255.f * 2.f;
            vertex3.color.y = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i) * 3] + 1) / 255.f * 2.f;
            vertex3.color.z = static_cast<int>(colorMap[((j + 1) * colorMapWidth + i) * 3] + 0) / 255.f * 2.f;
            vertex3.color.w = 1.f;

            m_indices.emplace_back(static_cast<uint16_t>(index0));
            m_indices.emplace_back(static_cast<uint16_t>(index1));
            m_indices.emplace_back(static_cast<uint16_t>(index2));
            m_indices.emplace_back(static_cast<uint16_t>(index0));
            m_indices.emplace_back(static_cast<uint16_t>(index2));
            m_indices.emplace_back(static_cast<uint16_t>(index3));
        }
    }

    CalculateNormal(m_vertices, m_indices);

    CreateBuffer(device, m_vertices, D3D11_BIND_VERTEX_BUFFER, &m_vertexBuffer);
    CreateBuffer(device, m_indices, D3D11_BIND_INDEX_BUFFER, &m_indexBuffer);
}


Terrain::~Terrain()
{
    m_states.reset();
    m_inputLayout.Reset();
    m_texture.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_terrainVertexShader.Reset();
    m_terrainPixelShader.Reset();
    m_reflectionMatrixBuffer.Reset();
    m_reflectionLightBuffer.Reset();
    m_clipPlaneConstBuffer.Reset();
}

void Terrain::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj,
    const DirectX::SimpleMath::Plane& clipPlane)
{
    m_reflectionMatrixBufferData.worldMatrix = world;
    m_reflectionMatrixBufferData.viewMatrix = view;
    m_reflectionMatrixBufferData.projectMatrix = proj;
    deviceContext->UpdateSubresource(m_reflectionMatrixBuffer.Get(), 0, nullptr, &m_reflectionMatrixBufferData, 0, 0);

    m_clipPlaneConstBufferData.clipPlane.x = clipPlane.x;
    m_clipPlaneConstBufferData.clipPlane.y = clipPlane.y;
    m_clipPlaneConstBufferData.clipPlane.z = clipPlane.z;
    m_clipPlaneConstBufferData.clipPlane.w = clipPlane.w;
    deviceContext->UpdateSubresource(m_clipPlaneConstBuffer.Get(), 0, nullptr, &m_clipPlaneConstBufferData, 0, 0);

    deviceContext->VSSetShader(m_terrainVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_terrainPixelShader.Get(), nullptr, 0);

    deviceContext->VSSetConstantBuffers(0, 1, m_reflectionMatrixBuffer.GetAddressOf());
    deviceContext->VSSetConstantBuffers(1, 1, m_clipPlaneConstBuffer.GetAddressOf());
    deviceContext->PSSetConstantBuffers(0, 1, m_reflectionLightBuffer.GetAddressOf());

    deviceContext->OMSetBlendState(m_states->AlphaBlend(), Colors::White, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    deviceContext->RSSetState(m_states->CullCounterClockwise());

    ID3D11SamplerState* samplers[] = { m_states->LinearWrap() };
    deviceContext->PSSetSamplers(0, 1, samplers);

    deviceContext->PSSetShaderResources(0, 1, m_texture.GetAddressOf());

    deviceContext->IASetInputLayout(m_inputLayout.Get());

    // Set the vertex and index buffer.
    auto vertexBuffer = m_vertexBuffer.Get();
    UINT vertexStride = sizeof(VertexType);
    UINT vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->DrawIndexed(static_cast<UINT>(m_indices.size()), 0, 0);
}
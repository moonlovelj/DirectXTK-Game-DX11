#include "pch.h"
#include "Water.h"
#include "Common.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Water::Water(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext, float waterHeight, float waterRadius)
    :m_waterHeight(waterHeight),
    m_waterRadius(waterRadius)
{
    deviceContext;

    m_waterBufferData.waterTranslation = 0.f;
    m_waterBufferData.reflectRefractScale = 0.03f;
    m_waterBufferData.lightDirection = { 0.f, -1.f, -2.f };
    m_waterBufferData.specularShininess = 200.f;

    m_camNormBufferData.normalMapTilingX = 0.1f;
    m_camNormBufferData.normalMapTilingY = 0.2f;


    m_vertices.resize(4);
    m_indices.resize(6);

    m_vertices[0].position = { -waterRadius, waterHeight, -waterRadius };
    m_vertices[0].SetColor(DirectX::XMFLOAT4{ 0.f, 0.8f, 1.f, 1.f });
    m_vertices[0].textureCoordinate = { 0.f, 0.f };

    m_vertices[1].position = { waterRadius - 1.f, waterHeight, -waterRadius };
    m_vertices[1].SetColor(DirectX::XMFLOAT4{ 0.f, 0.8f, 1.f, 1.f });
    m_vertices[1].textureCoordinate = { 1.f, 0.f };

    m_vertices[2].position = { waterRadius - 1.f, waterHeight, waterRadius - 1.f };
    m_vertices[2].SetColor(DirectX::XMFLOAT4{ 0.f, 0.8f, 1.f, 1.f });
    m_vertices[2].textureCoordinate = { 1.f, 1.f };

    m_vertices[3].position = { -waterRadius, waterHeight, waterRadius - 1.f };
    m_vertices[3].SetColor(DirectX::XMFLOAT4{ 0.f, 0.8f, 1.f, 1.f });
    m_vertices[3].textureCoordinate = { 0.f, 1.f };

    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 0;
    m_indices[4] = 2;
    m_indices[5] = 3;

    CalculateNormal(m_vertices, m_indices);
    CalculateTangent(m_vertices, m_indices);

    D3D11_BUFFER_DESC vertexBufferDesc{ static_cast<UINT>(m_vertices.size() * sizeof(VertexType)), D3D11_USAGE_DEFAULT , D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
    vertexDataDesc.pSysMem = m_vertices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, &m_vertexBuffer));

    D3D11_BUFFER_DESC indexBufferDesc{ static_cast<UINT>(m_indices.size() * sizeof(uint16_t)), D3D11_USAGE_DEFAULT , D3D11_BIND_INDEX_BUFFER };
    D3D11_SUBRESOURCE_DATA indexDataDesc = {};
    indexDataDesc.pSysMem = m_indices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexDataDesc, &m_indexBuffer));

    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"waternormal.dds", resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf()));

    auto blobVertex = DX::ReadData(L"WaterVertexShader.cso");
    DX::ThrowIfFailed(device->CreateVertexShader(blobVertex.data(), blobVertex.size(),
        nullptr, m_waterVertexShader.ReleaseAndGetAddressOf()));

    auto blobPixel = DX::ReadData(L"WaterPixelShader.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(blobPixel.data(), blobPixel.size(),
        nullptr, m_waterPixelShader.ReleaseAndGetAddressOf()));

    DX::ThrowIfFailed(
        device->CreateInputLayout(VertexType::InputElements,
            VertexType::InputElementCount,
            blobVertex.data(), blobVertex.size(),
            m_inputLayout.ReleaseAndGetAddressOf()));

    CD3D11_BUFFER_DESC constDesc(sizeof(MatrixBufferData), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_matrixBuffer)
    );

    constDesc.ByteWidth = sizeof(ReflectionBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_reflectBuffer)
    );

    constDesc.ByteWidth = sizeof(WaterBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_waterBuffer)
    );

    constDesc.ByteWidth = sizeof(CamNormBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_camNormBuffer)
    );

    m_states = std::make_unique<DirectX::CommonStates>(device);
}


Water::~Water()
{
    m_states.reset();
    m_inputLayout.Reset();
    m_waterVertexShader.Reset();
    m_waterPixelShader.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_texture.Reset();
    m_matrixBuffer.Reset();
    m_reflectBuffer.Reset();
    m_waterBuffer.Reset();
    m_camNormBuffer.Reset();
}

void Water::Update(float elapsedTime)
{
    m_waterBufferData.waterTranslation += elapsedTime * 0.1f;
    if (m_waterBufferData.waterTranslation > 1.f)
        m_waterBufferData.waterTranslation -= 1.f;
}

void Water::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world, 
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj,
    const DirectX::SimpleMath::Matrix& reflectView,
    ID3D11ShaderResourceView* reflectTexture, 
    ID3D11ShaderResourceView* refractTexture,
    const DirectX::SimpleMath::Vector3& camPos)
{
    m_matrixBufferData.worldMatrix = world;
    m_matrixBufferData.viewMatrix = view;
    m_matrixBufferData.projectMatrix = proj;
    deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, nullptr, &m_matrixBufferData, 0, 0);

    m_reflectBufferData.reflectionMatrix = reflectView;
    deviceContext->UpdateSubresource(m_reflectBuffer.Get(), 0, nullptr, &m_reflectBufferData, 0, 0);

    deviceContext->UpdateSubresource(m_waterBuffer.Get(), 0, nullptr, &m_waterBufferData, 0, 0);

    m_camNormBufferData.cameraPosition = camPos;
    deviceContext->UpdateSubresource(m_camNormBuffer.Get(), 0, nullptr, &m_camNormBufferData, 0, 0);

    deviceContext->VSSetShader(m_waterVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_waterPixelShader.Get(), nullptr, 0);

    deviceContext->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    deviceContext->VSSetConstantBuffers(1, 1, m_reflectBuffer.GetAddressOf());
    deviceContext->VSSetConstantBuffers(2, 1, m_camNormBuffer.GetAddressOf());

    deviceContext->PSSetConstantBuffers(0, 1, m_waterBuffer.GetAddressOf());


    deviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    deviceContext->RSSetState(m_states->CullCounterClockwise());

    ID3D11SamplerState* samplers[] = { m_states->LinearWrap() };
    deviceContext->PSSetSamplers(0, 1, samplers);

    deviceContext->PSSetShaderResources(0, 1, &refractTexture);
    deviceContext->PSSetShaderResources(1, 1, &reflectTexture);
    deviceContext->PSSetShaderResources(2, 1, m_texture.GetAddressOf());

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

DirectX::SimpleMath::Plane Water::GetWaterPlane() const
{
    return { 0.f, 1.f, 0.f, -m_waterHeight };
}

float Water::GetWaterPlaneHeight() const
{
    return m_waterHeight;
}

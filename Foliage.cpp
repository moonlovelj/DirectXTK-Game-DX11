#include "pch.h"
#include "Foliage.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Foliage::Foliage(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext)
{
    deviceContext;

    m_states = std::make_unique<CommonStates>(device);

    m_vertices.resize(1);
    m_vertices[0].position = { 0.f, 5.f, 0.f };

    size_t instanceCount = 32;
    m_instances.resize(instanceCount * instanceCount);
    for (size_t i = 0; i < instanceCount; i++)
    {
        for (size_t j = 0; j < instanceCount; j++)
        {
            m_instances[i*instanceCount + j] = { j * 8.f, 0.f, i*8.f };
        }
    }

    D3D11_BUFFER_DESC vertexBufferDesc{ static_cast<UINT>(m_vertices.size() * sizeof(VertexType)), D3D11_USAGE_DEFAULT , D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
    vertexDataDesc.pSysMem = m_vertices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, &m_vertexBuffer));

    D3D11_BUFFER_DESC instanceBufferDesc{ static_cast<UINT>(m_instances.size() * sizeof(InstanceType)), D3D11_USAGE_DYNAMIC , D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE };
    D3D11_SUBRESOURCE_DATA instanceDataDesc = {};
    instanceDataDesc.pSysMem = m_instances.data();
    DX::ThrowIfFailed(device->CreateBuffer(&instanceBufferDesc, &instanceDataDesc, &m_instanceBuffer));

    CD3D11_BUFFER_DESC constDesc(sizeof(MatrixBufferData), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_matrixBuffer)
    );

    constDesc.ByteWidth = sizeof(FoliageBufferData);
    DX::ThrowIfFailed(
        device->CreateBuffer(&constDesc, nullptr, &m_foliageBuffer)
    );

    auto blobVertex = DX::ReadData(L"FoliageVertexShader.cso");
    DX::ThrowIfFailed(device->CreateVertexShader(blobVertex.data(), blobVertex.size(),
        nullptr, m_foliageVertexShader.ReleaseAndGetAddressOf()));

    auto blobGeometry = DX::ReadData(L"FoliageGeometryShader.cso");
    DX::ThrowIfFailed(device->CreateGeometryShader(blobGeometry.data(), blobGeometry.size(),
        nullptr, m_foliageGeometryShader.ReleaseAndGetAddressOf()));

    auto blobPixel = DX::ReadData(L"FoliagePixelShader.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(blobPixel.data(), blobPixel.size(),
        nullptr, m_foliagePixelShader.ReleaseAndGetAddressOf()));

    const D3D11_INPUT_ELEMENT_DESC InputElements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    1, DXGI_FORMAT_R32G32B32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    DX::ThrowIfFailed(
        device->CreateInputLayout(InputElements,
            3,
            blobVertex.data(), blobVertex.size(),
            m_inputLayout.ReleaseAndGetAddressOf()));

    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"tree.dds", resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf()));
}


Foliage::~Foliage()
{
    m_states.reset();
    m_inputLayout.Reset();
    m_foliageVertexShader.Reset();
    m_foliagePixelShader.Reset();
    m_foliageGeometryShader.Reset();
    m_vertexBuffer.Reset();
    m_instanceBuffer.Reset();
    m_matrixBuffer.Reset();
    m_foliageBuffer.Reset();
    m_texture.Reset();
}

void Foliage::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world, 
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj,
    const DirectX::SimpleMath::Vector3& cameraPosition)
{
    m_matrixBufferData.worldMatrix = world;
    m_matrixBufferData.viewMatrix = view;
    m_matrixBufferData.projectMatrix = proj;
    deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, nullptr, &m_matrixBufferData, 0, 0);

    m_foliageBufferData.cameraPosition = cameraPosition;
    m_foliageBufferData.treeBillWidth = 2.5f;
    m_foliageBufferData.treeBillHeight = 4.f;
    deviceContext->UpdateSubresource(m_foliageBuffer.Get(), 0, nullptr, &m_foliageBufferData, 0, 0);

    deviceContext->VSSetShader(m_foliageVertexShader.Get(), nullptr, 0);
    deviceContext->GSSetShader(m_foliageGeometryShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_foliagePixelShader.Get(), nullptr, 0);

    deviceContext->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    deviceContext->GSSetConstantBuffers(0, 1, m_foliageBuffer.GetAddressOf());
    deviceContext->GSSetConstantBuffers(1, 1, m_matrixBuffer.GetAddressOf());

    deviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    deviceContext->RSSetState(m_states->CullNone());

    ID3D11SamplerState* samplers[] = { m_states->LinearClamp() };
    deviceContext->PSSetSamplers(0, 1, samplers);

    deviceContext->PSSetShaderResources(0, 1, m_texture.GetAddressOf());

    deviceContext->IASetInputLayout(m_inputLayout.Get());

    // Set the vertex and index buffer.
    UINT strides[2] = { sizeof(VertexType), sizeof(InstanceType) };
    UINT offsets[2] = { 0, 0 };
    ID3D11Buffer* vertBillInstBuffers[2] = { m_vertexBuffer.Get(), m_instanceBuffer.Get() };
    deviceContext->IASetVertexBuffers(0, 2, vertBillInstBuffers, strides, offsets);
    
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    deviceContext->DrawInstanced(m_vertices.size(), m_instances.size(), 0, 0);

    deviceContext->GSSetShader(nullptr, nullptr, 0);
}

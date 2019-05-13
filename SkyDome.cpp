#include "pch.h"
#include "SkyDome.h"

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

SkyDome::SkyDome(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext)
{
    deviceContext;

    m_states = std::make_unique<CommonStates>(device);

    m_effectSky = std::make_unique<BasicEffect>(device);
    m_effectSky->SetVertexColorEnabled(true);
    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effectSky->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    DX::ThrowIfFailed(
        device->CreateInputLayout(VertexType::InputElements,
            VertexType::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_inputLayoutSky.ReleaseAndGetAddressOf()));
    
    std::vector<GeometricPrimitive::VertexType> vertices;
    GeometricPrimitive::CreateSphere(vertices, m_indicesSky, 2, 20);
    m_verticesSky.resize(vertices.size());
    Color apexColor(0.f, 0.15f, 0.66f, 1.f);
    Color centerColor(0.81f, 0.38f, 0.66f, 1.f);
    for (size_t i = 0; i < m_verticesSky.size(); i++)
    {
        m_verticesSky[i].position.x = vertices[i].position.x;
        m_verticesSky[i].position.y = vertices[i].position.y;
        m_verticesSky[i].position.z = vertices[i].position.z;
        Color finalColor = centerColor * (1.f - std::abs(m_verticesSky[i].position.y)) + apexColor * std::abs(m_verticesSky[i].position.y);
        m_verticesSky[i].color.x = finalColor.R();
        m_verticesSky[i].color.y = finalColor.G();
        m_verticesSky[i].color.z = finalColor.B();
        m_verticesSky[i].color.w = finalColor.A();
    }
    CreateBuffer(device, m_verticesSky, D3D11_BIND_VERTEX_BUFFER, &m_vertexBufferSky);
    CreateBuffer(device, m_indicesSky, D3D11_BIND_INDEX_BUFFER, &m_indexBufferSky);
}


SkyDome::~SkyDome()
{
    m_states.reset();
    m_effectSky.reset();
    m_inputLayoutSky.Reset();
    m_vertexBufferSky.Reset();
    m_indexBufferSky.Reset();
}

void SkyDome::Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
    m_effectSky->SetWorld(world);
    m_effectSky->SetView(view);
    m_effectSky->SetProjection(proj);
    m_effectSky->Apply(deviceContext);

    deviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
    deviceContext->RSSetState(m_states->CullNone());

    ID3D11SamplerState* samplers[] = { m_states->LinearWrap() };
    deviceContext->PSSetSamplers(0, 1, samplers);

    deviceContext->IASetInputLayout(m_inputLayoutSky.Get());

    // Set the vertex and index buffer.s
    auto vertexBuffer = m_vertexBufferSky.Get();
    UINT vertexStride = sizeof(VertexType);
    UINT vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetIndexBuffer(m_indexBufferSky.Get(), DXGI_FORMAT_R16_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->DrawIndexed(static_cast<UINT>(m_indicesSky.size()), 0, 0);
}
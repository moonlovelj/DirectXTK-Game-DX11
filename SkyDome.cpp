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

struct CloudParams
{
    float firstTranslationX;
    float firstTranslationZ;
    float secondTranslationX;
    float secondTranslationZ;
    float brightness;
    uint8_t padding[12];
};

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
    
    m_effectCloud = std::make_unique<BasicEffect>(device);
    m_effectCloud->SetTextureEnabled(true);
    m_effectCloud->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    
    DX::ThrowIfFailed(
        device->CreateInputLayout(CloudVertexType::InputElements,
            CloudVertexType::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_inputLayoutCloud.ReleaseAndGetAddressOf()));

    CD3D11_BUFFER_DESC cbDesc(sizeof(CloudParams), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_cloudParams.ReleaseAndGetAddressOf()));

    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"cloud001.dds", resource.GetAddressOf(), m_texture0Cloud.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"cloud002.dds", resource.GetAddressOf(), m_texture1Cloud.ReleaseAndGetAddressOf()));

    auto blob = DX::ReadData(L"Cloud.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(blob.data(), blob.size(),
        nullptr, m_cloudPS.ReleaseAndGetAddressOf()));

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

    // Create clouds
    float quadSize = m_cloudWidth / m_cloudResolution;
    float radius = m_cloudWidth / 2.f;
    float constant = (m_cloudTop - m_cloudBottom) / (radius * radius);
    float textureDelta = static_cast<float>(m_cloudTextureRepeat) / m_cloudResolution;

    m_verticesCloud.resize((m_cloudResolution + 1) * (m_cloudResolution + 1));
    for (int j = 0; j <= m_cloudResolution; j++)
    {
        for (int i = 0; i <= m_cloudResolution; i++)
        {
            // Calculate the vertex coordinates.
            auto positionX = (-0.5f * m_cloudWidth) + (i * quadSize);
            auto positionZ = (-0.5f * m_cloudWidth) + (j * quadSize);
            auto positionY = m_cloudTop - (constant * ((positionX * positionX) + (positionZ * positionZ)));

            // Calculate the texture coordinates.
            auto tu = i * textureDelta;
            auto tv = j * textureDelta;

            // Calculate the index into the sky plane array to add this coordinate.
            auto index = j * (m_cloudResolution + 1) + i;

            // Add the coordinates to the sky plane array.
            m_verticesCloud[index].position.x = positionX;
            m_verticesCloud[index].position.y = positionY;
            m_verticesCloud[index].position.z = positionZ;
            m_verticesCloud[index].textureCoordinate.x = tu;
            m_verticesCloud[index].textureCoordinate.y = tv;
        }
    }

    for (int j = 0; j < m_cloudResolution; j++)
    {
        for (int i = 0; i < m_cloudResolution; i++)
        {
            auto index0 = j * m_cloudResolution + i;
            auto index1 = j * m_cloudResolution + i + 1;
            auto index2 = (j + 1) * m_cloudResolution + i + 1;
            auto index3 = (j + 1) * m_cloudResolution + i;
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index0));
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index1));
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index2));
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index0));
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index2));
            m_indicesCloud.emplace_back(static_cast<uint16_t>(index3));
        }
    }

    CreateBuffer(device, m_verticesCloud, D3D11_BIND_VERTEX_BUFFER, &m_vertexBufferCloud);
    CreateBuffer(device, m_indicesCloud, D3D11_BIND_INDEX_BUFFER, &m_indexBufferCloud);
}


SkyDome::~SkyDome()
{
    m_states.reset();
    m_effectSky.reset();
    m_inputLayoutSky.Reset();
    m_vertexBufferSky.Reset();
    m_indexBufferSky.Reset();
    m_inputLayoutCloud.Reset();
    m_vertexBufferCloud.Reset();
    m_indexBufferCloud.Reset();
    m_texture0Cloud.Reset();
    m_texture1Cloud.Reset();
    m_effectCloud.reset();
    m_cloudPS.Reset();
    m_cloudParams.Reset();
}

void SkyDome::Update(float elapsedTime)
{
    elapsedTime;
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

    // Render cloud
    m_effectCloud->SetWorld(world);
    m_effectCloud->SetView(view);
    m_effectCloud->SetProjection(proj);
    m_effectCloud->Apply(deviceContext);

    deviceContext->PSSetShader(m_cloudPS.Get(), nullptr, 0);
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DX::ThrowIfFailed(deviceContext->Map(m_cloudParams.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    CloudParams para;
    para.firstTranslationX = 0.f;
    para.firstTranslationZ = 0.f;
    para.secondTranslationX = 0.f;
    para.secondTranslationZ = 0.f;
    para.brightness = 0.65f;
    *static_cast<CloudParams*>(mappedResource.pData) = para;
    deviceContext->Unmap(m_cloudParams.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, m_cloudParams.GetAddressOf());
    deviceContext->PSSetShaderResources(0, 1, m_texture0Cloud.GetAddressOf());
    deviceContext->PSSetShaderResources(1, 1, m_texture1Cloud.GetAddressOf());
    
    deviceContext->OMSetBlendState(m_states->Additive(), Color{0,0,0,0}, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
    deviceContext->RSSetState(m_states->CullNone());

    deviceContext->PSSetSamplers(0, 1, samplers);
    deviceContext->IASetInputLayout(m_inputLayoutCloud.Get());

    vertexBuffer = m_vertexBufferCloud.Get();
    vertexStride = sizeof(CloudVertexType);
    vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetIndexBuffer(m_indexBufferCloud.Get(), DXGI_FORMAT_R16_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->DrawIndexed(static_cast<UINT>(m_indicesCloud.size()), 0, 0);
}
#pragma once
class SkyDome
{
    using VertexType = DirectX::VertexPositionColor;

public:
    SkyDome(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~SkyDome();

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

private:
    std::unique_ptr<DirectX::CommonStates>       m_states;
    std::unique_ptr<DirectX::BasicEffect>        m_effectSky;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_inputLayoutSky;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_vertexBufferSky;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_indexBufferSky;
    std::vector<VertexType>                      m_verticesSky;
    std::vector<uint16_t>                        m_indicesSky;
};


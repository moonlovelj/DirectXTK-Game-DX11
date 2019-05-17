#pragma once
class SkyDome
{
    using VertexType = DirectX::VertexPositionColor;
    using CloudVertexType = DirectX::VertexPositionTexture;

public:
    SkyDome(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~SkyDome();

    void Update(float elapsedTime);

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

    // Cloud
    int                                          m_cloudResolution = 10;
    float                                        m_cloudWidth = 10.0f;
    float                                        m_cloudTop = 0.5f;
    float                                        m_cloudBottom = 0.0f;
    int                                          m_cloudTextureRepeat = 4;
    float                                        m_cloudBrightness = 0.65f;
    DirectX::SimpleMath::Vector2                 m_cloudTexture0TranslationSpeed{ 0.0003f, 0.f };
    DirectX::SimpleMath::Vector2                 m_cloudTexture1TranslationSpeed{ 0.00015f, 0.f };
    DirectX::SimpleMath::Vector2                 m_cloudTexture0Translation{ 0.f, 0.f };
    DirectX::SimpleMath::Vector2                 m_cloudTexture1Translation{ 0.f, 0.f };
    std::unique_ptr<DirectX::BasicEffect>        m_effectCloud;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_inputLayoutCloud;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_vertexBufferCloud;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_indexBufferCloud;

    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_cloudParams;       
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_cloudPS;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture0Cloud;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture1Cloud;
    std::vector<CloudVertexType>                 m_verticesCloud;
    std::vector<uint16_t>                        m_indicesCloud;
};


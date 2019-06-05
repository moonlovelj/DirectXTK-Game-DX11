#pragma once
class Water
{
public:
    Water(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext, float waterHeight, float waterRadius);
    ~Water();

    using VertexType = DirectX::VertexPositionNormalTangentColorTexture;

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

private:
    std::unique_ptr<DirectX::CommonStates>          m_states;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_waterVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_waterPixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

    std::vector<VertexType>                         m_vertices;
    std::vector<uint16_t>                           m_indices;

    MatrixBufferData                                m_matrixBufferData;

    float                                           m_waterHeight;
    float                                           m_waterRadius;
};


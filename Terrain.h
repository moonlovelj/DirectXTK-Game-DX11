#pragma once

class Terrain
{
public:
    Terrain(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~Terrain();

    using VertexType = DirectX::VertexPositionNormalColorTexture;

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj, 
        const DirectX::SimpleMath::Plane& clipPlane);

private:
    static const int                                m_terrainMaxHeight = 32;
    static const int                                m_terrainTextureRepeat = 4;

    int                                             m_terrainWidth;
    int                                             m_terrainHeight;

    std::unique_ptr<DirectX::CommonStates>          m_states;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_terrainVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_terrainPixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_reflectionMatrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_reflectionLightBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_clipPlaneConstBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

    MatrixBufferData                                m_reflectionMatrixBufferData;
    ReflectionLightBufferData                       m_reflectionLightBufferData;
    ClipPlaneConstBufferData                        m_clipPlaneConstBufferData;

    std::vector<VertexType>                         m_vertices;
    std::vector<uint16_t>                           m_indices;
};


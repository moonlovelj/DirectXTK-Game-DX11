#pragma once
class Foliage
{
public:
    using VertexType = DirectX::VertexPositionTexture;
    using InstanceType = DirectX::SimpleMath::Vector3;

    Foliage(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~Foliage();

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj, 
        const DirectX::SimpleMath::Vector3& cameraPosition);

private:

    std::unique_ptr<DirectX::CommonStates>          m_states;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_foliageVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_foliagePixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_foliageGeometryShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_instanceBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_foliageBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

    MatrixBufferData                                m_matrixBufferData;
    FoliageBufferData                               m_foliageBufferData;

    std::vector<VertexType>                         m_vertices;
    std::vector<InstanceType>                       m_instances;
};


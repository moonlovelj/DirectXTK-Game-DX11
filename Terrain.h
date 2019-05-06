#pragma once
class Terrain
{
public:
    Terrain(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~Terrain();

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

private:
    int                                             m_terrainWidth;
    int                                             m_terrainHeight;

    std::unique_ptr<DirectX::CommonStates>          m_states;
    std::unique_ptr<DirectX::BasicEffect>           m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
};


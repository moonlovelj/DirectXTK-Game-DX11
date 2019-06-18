#pragma once
class Foliage
{
public:
    using VertexType = DirectX::VertexPositionTexture;
    using InstanceType = DirectX::SimpleMath::Vector3;

    Foliage(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext);
    ~Foliage();

    void Render(ID3D11DeviceContext1* deviceContext, const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

private:

    std::vector<VertexType>                         m_vertices;
    std::vector<InstanceType>                       m_instances;
};


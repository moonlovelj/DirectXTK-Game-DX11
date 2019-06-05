#pragma once

#include <windef.h>

#include <fstream>
#include <vector>

typedef struct tagBITMAPFILEHEADER {
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

bool ReadBitmap(const std::string& fileName, std::vector<uint8_t>& data, int& width, int& height);

template <typename T>
void CalculateNormal(std::vector<T>& vertices, const std::vector<uint16_t>& indices)
{
    if (indices.size() % 3 != 0)
        throw std::exception("CalculateNormal is suitable for triangles only");

    std::vector<DirectX::SimpleMath::Vector3> normals(vertices.size());
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        const auto& vertex0 = Vector3(vertices[indices[i + 0]].position);
        const auto& vertex1 = Vector3(vertices[indices[i + 1]].position);
        const auto& vertex2 = Vector3(vertices[indices[i + 2]].position);
        auto normal = (vertex2 - vertex0).Cross(vertex1 - vertex0);
        normals[indices[i + 0]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    for (auto& normal: normals)
    {
        normal.Normalize();
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
        vertices[i].normal.x = normals[i].x;
        vertices[i].normal.y = normals[i].y;
        vertices[i].normal.z = normals[i].z;
    }
}

template <typename T>
void CalculateTangent(std::vector<T>& vertices, const std::vector<uint16_t>& indices)
{
    if (indices.size() % 3 != 0)
        throw std::exception("CalculateNormal is suitable for triangles only");
    
    std::vector<DirectX::SimpleMath::Vector3> tangents(vertices.size());
    std::vector<DirectX::SimpleMath::Vector3> bitangents(vertices.size());
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        auto index0 = indices[i + 0];
        auto index1 = indices[i + 1];
        auto index2 = indices[i + 2];

        const auto& v0 = vertices[index0].position;
        const auto& v1 = vertices[index1].position;
        const auto& v2 = vertices[index2].position;

        const auto& w0 = vertices[index0].textureCoordinate;
        const auto& w1 = vertices[index1].textureCoordinate;
        const auto& w2 = vertices[index2].textureCoordinate;

        float x1 = v1.x - v0.x;
        float x2 = v2.x - v0.x;
        float y1 = v1.y - v0.y;
        float y2 = v2.y - v0.y;
        float z1 = v1.z - v0.z;
        float z2 = v2.z - v0.z;

        float s1 = w1.x - w0.x;
        float s2 = w2.x - w0.x;
        float t1 = w1.y - w0.y;
        float t2 = w2.y - w0.y;

        float r = 1.f / (s1 * t2 - s2 * t1);
        DirectX::SimpleMath::Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        DirectX::SimpleMath::Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        tangents[index0] += sdir;
        tangents[index1] += sdir;
        tangents[index2] += sdir;

        bitangents[index0] += tdir;
        bitangents[index1] += tdir;
        bitangents[index2] += tdir;
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
        const auto& t = tangents[i];
        DirectX::SimpleMath::Vector3 n(vertices[i].normal);

        // Gram-Schmidt orthogonalize
        auto tangent = (t - n * n.Dot(t));
        tangent.Normalize();

        // Calculate handedness
        auto w = (n.Cross(t).Dot(bitangents[i]) < 0.f) ? -1.f : 1.f;
        vertices[i].tangent.x = tangent.x;
        vertices[i].tangent.y = tangent.y;
        vertices[i].tangent.z = tangent.z;
        vertices[i].tangent.w = w;
    }
}
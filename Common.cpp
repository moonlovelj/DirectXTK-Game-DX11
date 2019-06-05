#include "pch.h"
#include "Common.hpp"

bool ReadBitmap(const std::string& fileName, std::vector<uint8_t>& data, int& width, int& height)
{
    std::ifstream fs(fileName, std::ios::binary | std::ios::in);
    if (!fs.is_open())
        return false;

    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;
    char buffer[2];
    fs.read(buffer, 2);
    fs.read((char*)&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
    fs.read((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));
    if (bitmapInfoHeader.biBitCount != 24)
        return false;

    width = bitmapInfoHeader.biWidth;
    height = bitmapInfoHeader.biHeight;
    data.resize(bitmapInfoHeader.biWidth * bitmapInfoHeader.biHeight * 3);
    fs.seekg(bitmapFileHeader.bfOffBits);
    fs.read((char*)&data[0], bitmapInfoHeader.biWidth * bitmapInfoHeader.biHeight * 3);
    fs.close();
    return true;
}
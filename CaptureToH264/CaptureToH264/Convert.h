#pragma once
#include <string>

class Convert
{
public:
    static int Rgb24ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char** bmpBuf);
    static int Rgb32ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char** bmpBuf);
    static int Rgb24ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf);
    static bool Rgb32ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char** yuvBuf, const int yuvBufferSize);
    static bool YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height);
};


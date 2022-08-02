#pragma once

class Convert
{
public:
    static int Rgb24ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char** bmpBuf);
    static int Rgb32ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char** bmpBuf);
    static int Rgb24ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf);

    static bool Rgb32ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char** yuvBuf, const int yuvBufferSize);
};


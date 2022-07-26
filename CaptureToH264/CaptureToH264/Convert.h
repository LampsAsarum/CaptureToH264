#pragma once

class Convert
{
public:
    static bool Rgb24ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char* bmpBuf);
    static bool Rgb32ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char* bmpBuf);
    static bool Rgb24ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char* yuvBuf);
    static bool Rgb32ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char* yuvBuf);
};


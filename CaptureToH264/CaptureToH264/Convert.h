#pragma once

class Convert
{
public:
    static int Rgb24ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char** bmpBuf);
    static int Rgb32ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char** bmpBuf);
    static int Rgb24ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char* yuvBuf);
    static int Rgb32ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char** yuvBuf);
};


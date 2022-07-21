#pragma once

class SaveFile
{
public:
    static void Rgb24(const char* filename, unsigned char* rgbbuf, int width, int height);
    static void Rgb32(const char* filename, unsigned char* rgbbuf, int width, int height);
    static void Rgb24ToBmp(const char* filename, unsigned char* rgbbuf, int width, int height);
    static void Rgb32ToBmp(const char* filename, unsigned char* rgbbuf, int width, int height);
};


#pragma once
class SaveBmp
{
public:
    static void Rgb24ToBmp(const char* filename, unsigned char* rgbbuf, int width, int height);
};


#include <iostream>
#include <Windows.h>
#include "ConvertScale.h"
#include "DXGICapture.h"

void SaveFile(const char* fileName, unsigned char* buf, int size)
{
    FILE* fp = nullptr;
    fopen_s(&fp, fileName, "ab");
    if (!fp) return;
    fwrite(buf, size, 1, fp);
    fclose(fp);
}

void DXGICaptureRgb32(int width, int height)
{
    int rgb32Size = ConvertScale::GetImageSize(width, height, AV_PIX_FMT_BGRA, 16);
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];

    DXGICapture capture;
    ConvertScale scale;
    scale.Init(1920, 1080, AV_PIX_FMT_BGRA, 1920, 1080, AV_PIX_FMT_YUV420P);

    int yuv420Size = ConvertScale::GetImageSize(width, height, AV_PIX_FMT_YUV420P, 16);
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    char picName[128] = { 0 };

    for (int i = 0; i < 3; i++)
    {
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size))
        {
            snprintf(picName, sizeof(picName), "DXGIScreen%d.rgb", i);
            SaveFile(picName, rgbBuffer, rgb32Size);
            scale.Convert(rgbBuffer, yuvBuffer);
            snprintf(picName, sizeof(picName), "DXGIScreen%d.yuv", i);
            SaveFile(picName, yuvBuffer, yuv420Size);
        }
    }
    delete[] rgbBuffer; rgbBuffer = nullptr;
}

int main()
{
    DXGICaptureRgb32(1920, 1080);
    return 0;
}

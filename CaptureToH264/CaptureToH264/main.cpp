#include <iostream>
#include <Windows.h>
#include "GDICapture.h"
#include "DXGICapture.h"
#include "Convert.h"

void SaveFile(const char* fileName, unsigned char* buf, int size)
{
    FILE* fp = nullptr;
    fopen_s(&fp, fileName, "wb");
    if (!fp) return;
    fwrite(buf, size, 1, fp);
    fclose(fp);
}

void CaptureRgb24(int width, int height)
{
    int rgb24Size = width * height * 3;
    unsigned char* rgbBuffer = new unsigned char[rgb24Size];
    if (GDICapture::CaptureRgb24(&rgbBuffer, rgb24Size)) 
    {
        SaveFile("Screen24.rgb", rgbBuffer, rgb24Size);

        unsigned char* bmpBuffer = nullptr;
        int bmpSize = Convert::Rgb24ToBmp(rgbBuffer, width, height, &bmpBuffer);
        if (bmpSize > 0) {
            SaveFile("Screen24.bmp", bmpBuffer, bmpSize);
        }

        int yuvSize = width * height * 3 / 2;
        unsigned char* yuvBuffer = (unsigned char*)malloc(width * height * 3 / 2);
        if (Convert::Rgb24ToYUV420(rgbBuffer, width, height, yuvBuffer)) {
            SaveFile("Screen24.yuv", yuvBuffer, yuvSize);
        }
    }
}

void CaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    if (GDICapture::CaptureRgb32(&rgbBuffer, rgb32Size))
    {
        SaveFile("Screen32.rgb", rgbBuffer, rgb32Size);
        
        unsigned char* bmpBuffer = nullptr;
        int bmpSize = Convert::Rgb32ToBmp(rgbBuffer, width, height, &bmpBuffer);
        if (bmpSize > 0) {
            SaveFile("Screen32.bmp", bmpBuffer, bmpSize);
        }

        unsigned char* yuvBuffer = nullptr;
        int yuvSize = Convert::Rgb32ToYUV420(rgbBuffer, width, height, &yuvBuffer);
        if (yuvSize > 0) {
            SaveFile("Screen32.yuv", yuvBuffer, yuvSize);
        }
    }
}

void DXGICaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    DXGICapture capture;
    for (int i = 0; i < 3; i++) 
    {
        if (capture.CaptureRgb32(&rgbBuffer, rgb32Size)) {
            char picName[128] = { 0 };
            snprintf(picName, sizeof(picName), "DXGIScreen%d.rgb", i);
            SaveFile(picName, rgbBuffer, rgb32Size);

            unsigned char* bmpBuffer = nullptr;
            int bmpSize = Convert::Rgb32ToBmp(rgbBuffer, width, height, &bmpBuffer);
            if (bmpSize > 0) {
                snprintf(picName, sizeof(picName), "DXGIScreen%d.bmp", i);
                SaveFile(picName, bmpBuffer, bmpSize);
            }

            unsigned char* yuvBuffer = nullptr;
            int yuvSize = Convert::Rgb32ToYUV420(rgbBuffer, width, height, &yuvBuffer);
            if (yuvSize > 0) {
                snprintf(picName, sizeof(picName), "DXGIScreen%d.yuv", i);
                SaveFile(picName, yuvBuffer, yuvSize);
            }
        }
        Sleep(1000);
    }
}

int main()
{
    RECT rect;
    HWND hWnd = GetDesktopWindow();
    GetWindowRect(hWnd, &rect);
    CloseWindow(hWnd);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    CaptureRgb24(width, height);
    CaptureRgb32(width, height); 

    DXGICaptureRgb32(width, height);

    return 0;
}
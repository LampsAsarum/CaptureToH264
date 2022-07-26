#include <iostream>
#include <Windows.h>
#include "GDICapture.h"
#include "Convert.h"
#include "libyuv/convert_from_argb.h"

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
    if (GDICapture::CaptureRgb24(rgbBuffer, rgb24Size)) {
        SaveFile("Screen24.rgb", rgbBuffer, rgb24Size);

        int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb24Size;
        unsigned char* bmpBuffer = new unsigned char[bmpSize];
        if (Convert::Rgb24ToBmp(rgbBuffer, width, height, bmpBuffer)) {
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
    if (GDICapture::CaptureRgb32(rgbBuffer, rgb32Size)) {
        SaveFile("Screen32.rgb", rgbBuffer, rgb32Size);
        int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size;
        unsigned char* bmpBuffer = new unsigned char[bmpSize];
        if (Convert::Rgb32ToBmp(rgbBuffer, width, height, bmpBuffer)) {
            SaveFile("Screen32.bmp", bmpBuffer, bmpSize);
        }

        int yuvSize = width * height * 3 / 2;
        unsigned char* yuvBuffer = (unsigned char*)malloc(width * height * 3 / 2);
        if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuvBuffer)) {
            SaveFile("Screen32.yuv", yuvBuffer, yuvSize);
        }
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

    return 0;
}
#include <iostream>
#include <Windows.h>
#include "GDICapture.h"
#include "SaveFile.h"

int main()
{
    RECT rect;
    HWND hWnd = GetDesktopWindow();
    GetWindowRect(hWnd, &rect);
    CloseWindow(hWnd);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int rgb24Size = width * height * 3;
    unsigned char* rgbBuffer = new unsigned char[rgb24Size];
    if (GDICapture::CaptureRgb24(rgbBuffer, rgb24Size)) {
        SaveFile::Rgb24("Screen24.rgb", rgbBuffer, width, height);
        SaveFile::Rgb24ToBmp("Screen24.bmp", rgbBuffer, width, height);
    }

    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer32 = new unsigned char[rgb32Size];
    if (GDICapture::CaptureRgb32(rgbBuffer32, rgb32Size)) {
        SaveFile::Rgb32("Screen32.rgb", rgbBuffer32, width, height);
        SaveFile::Rgb32ToBmp("Screen32.bmp", rgbBuffer32, width, height);
    }

    return 0;
}
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

    unsigned char* rgbBuffer = new unsigned char[width * height * 3];
    GDICapture::CaptureRgb24(rgbBuffer);

    SaveFile::Rgb24("Screen.rgb", rgbBuffer, width, height);

    SaveFile::Rgb24ToBmp("Screen.bmp", rgbBuffer, width, height);

    return 0;
}
#include "GDICapture.h"
#include <Windows.h>

void GDICapture::CaptureRgb24(unsigned char* rgbBuffer)
{
    HWND hDesk = GetDesktopWindow();
    HDC hScreen = GetDC(hDesk);
    int width = GetDeviceCaps(hScreen, HORZRES);
    int height = GetDeviceCaps(hScreen, VERTRES);

    HDC hdcMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);

    BITMAPINFOHEADER bmi = { 0 };
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 24;
    bmi.biWidth = width;
    bmi.biHeight = height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = width * height;

    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

    GetDIBits(hdcMem, hBitmap, 0, height, rgbBuffer, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    DeleteDC(hdcMem);
    ReleaseDC(hDesk, hScreen);
    DeleteObject(hBitmap);
}

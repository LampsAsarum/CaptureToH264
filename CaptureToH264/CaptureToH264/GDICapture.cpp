#include "GDICapture.h"
#include <Windows.h>

// API介绍：https://blog.csdn.net/zjy1175044232/article/details/111476009

bool GDICapture::CaptureRgb24(unsigned char* rgbBuffer, int bufferSize)
{
    HDC hdcScreen = GetDC(HWND_DESKTOP); //获取桌面DC
    //DC就是设备上下文，就是当前的这个窗体的一些属性，例如使用的位图、画笔、画刷等
    int width = GetDeviceCaps(hdcScreen, HORZRES); //获取屏幕水平像素数
    int height = GetDeviceCaps(hdcScreen, VERTRES);//获取屏幕垂直像素数

    if (bufferSize < width * height * 3) {
        return false;
    }

    HDC hdcMemory = CreateCompatibleDC(hdcScreen);//创建一个与桌面DC兼容的内存DC
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);//建立一个与桌面DC兼容的位图

    BITMAPINFOHEADER bih = { 0 };
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = width * height * 3;

    SelectObject(hdcMemory, hBitmap);//为标识的设备环境hdcMemory选择一个对象hBitmap
    BitBlt(hdcMemory, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY); //将hdcScreen的位图复制到hdcMemory

    GetDIBits(hdcMemory, hBitmap, 0, height, rgbBuffer, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

    ReleaseDC(HWND_DESKTOP, hdcMemory);
    DeleteDC(hdcMemory);
    ReleaseDC(HWND_DESKTOP, hdcScreen);
    DeleteObject(hBitmap);
}

bool GDICapture::CaptureRgb32(unsigned char* rgbBuffer, int bufferSize)
{
    HDC hdcScreen = GetDC(HWND_DESKTOP);
    int width = GetDeviceCaps(hdcScreen, HORZRES);
    int height = GetDeviceCaps(hdcScreen, VERTRES);

    if (bufferSize < width * height * 4) {
        return false;
    }

    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    HBITMAP bitmap = CreateCompatibleBitmap(hdcScreen, width, height);

    SelectObject(hdcMemory, bitmap);
    BitBlt(hdcMemory, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    GetBitmapBits(bitmap, width * height * 4, rgbBuffer);

    ReleaseDC(HWND_DESKTOP, hdcMemory);
    DeleteDC(hdcMemory);
    ReleaseDC(HWND_DESKTOP, hdcScreen);
    DeleteObject(bitmap);
}

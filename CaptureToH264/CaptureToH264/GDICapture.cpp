#include "GDICapture.h"
#include <Windows.h>

// API���ܣ�https://blog.csdn.net/zjy1175044232/article/details/111476009

bool GDICapture::CaptureRgb24(unsigned char* rgbBuffer, int bufferSize)
{
    HDC hdcScreen = GetDC(HWND_DESKTOP); //��ȡ����DC
    //DC�����豸�����ģ����ǵ�ǰ����������һЩ���ԣ�����ʹ�õ�λͼ�����ʡ���ˢ��
    int width = GetDeviceCaps(hdcScreen, HORZRES); //��ȡ��Ļˮƽ������
    int height = GetDeviceCaps(hdcScreen, VERTRES);//��ȡ��Ļ��ֱ������

    if (bufferSize < width * height * 3) {
        return false;
    }

    HDC hdcMemory = CreateCompatibleDC(hdcScreen);//����һ��������DC���ݵ��ڴ�DC
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);//����һ��������DC���ݵ�λͼ

    BITMAPINFOHEADER bih = { 0 };
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = width * height * 3;

    SelectObject(hdcMemory, hBitmap);//Ϊ��ʶ���豸����hdcMemoryѡ��һ������hBitmap
    BitBlt(hdcMemory, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY); //��hdcScreen��λͼ���Ƶ�hdcMemory

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

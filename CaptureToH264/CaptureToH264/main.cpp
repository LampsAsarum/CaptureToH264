#include <iostream>
#include <Windows.h>
#include <vector>
#include "GDICapture.h"
#include "DXGICapture.h"
#include "Convert.h"
#include "X264Encoder.h"
#include "FFmpegDecoder.h"
#include "Direct3DRender.h"

#pragma warning(disable:4996)

void SaveFile(const char* fileName, unsigned char* buf, int size)
{
    FILE* fp = nullptr;
    fopen_s(&fp, fileName, "ab");
    if (!fp) return;
    fwrite(buf, size, 1, fp);
    fclose(fp);
}

void CaptureRgb24(int width, int height)
{
    int rgb24Size = width * height * 3;
    unsigned char* rgbBuffer = new unsigned char[rgb24Size];

    int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb24Size;
    unsigned char* bmpBuffer = new unsigned char[bmpSize];

    int yuvSize = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuvSize];

    if (GDICapture::CaptureRgb24(rgbBuffer, rgb24Size))
    {
        SaveFile("GDIScreen24.rgb", rgbBuffer, rgb24Size);

        if (Convert::Rgb24ToBmp(rgbBuffer, width, height, bmpBuffer, bmpSize)) {
            SaveFile("GDIScreen24.bmp", bmpBuffer, bmpSize);
        }

        if (Convert::Rgb24ToYUV420(rgbBuffer, width, height, yuvBuffer, yuvSize)) {
            SaveFile("GDIScreen24.yuv", yuvBuffer, yuvSize);
        }
    }

    delete[] rgbBuffer; rgbBuffer = nullptr;
    delete[] bmpBuffer; bmpBuffer = nullptr;
    delete[] yuvBuffer; yuvBuffer = nullptr;
}

void CaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];

    int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size;
    unsigned char* bmpBuffer = new unsigned char[bmpSize];

    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    if (GDICapture::CaptureRgb32(rgbBuffer, rgb32Size))
    {
        SaveFile("GDIScreen32.rgb", rgbBuffer, rgb32Size);

        if (Convert::Rgb32ToBmp(rgbBuffer, width, height, bmpBuffer, bmpSize)) {
            SaveFile("GDIScreen32.bmp", bmpBuffer, bmpSize);
        }

        if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuvBuffer, yuv420Size)) {
            SaveFile("GDIScreen32.yuv", yuvBuffer, yuv420Size);
        }
    }

    delete[] rgbBuffer; rgbBuffer = nullptr;
    delete[] bmpBuffer; bmpBuffer = nullptr;
    delete[] yuvBuffer; yuvBuffer = nullptr;
}

void DXGICaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];

    int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size;
    unsigned char* bmpBuffer = new unsigned char[bmpSize];

    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    DXGICapture capture;
    char picName[128] = { 0 };
    for (int i = 0; i < 3; i++)
    {
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size)) 
        {
            snprintf(picName, sizeof(picName), "DXGIScreen%d.rgb", i);
            SaveFile(picName, rgbBuffer, rgb32Size);

            if (Convert::Rgb32ToBmp(rgbBuffer, width, height, bmpBuffer, bmpSize)) {
                snprintf(picName, sizeof(picName), "DXGIScreen%d.bmp", i);
                SaveFile(picName, bmpBuffer, bmpSize);
            }

            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuvBuffer, yuv420Size)) {
                snprintf(picName, sizeof(picName), "DXGIScreen%d.yuv", i);
                SaveFile(picName, yuvBuffer, yuv420Size);
            }
        }
        Sleep(1000);
    }

    delete[] rgbBuffer; rgbBuffer = nullptr;
    delete[] bmpBuffer; bmpBuffer = nullptr;
    delete[] yuvBuffer; yuvBuffer = nullptr;
}

void DXGICaptureRgb32ToYuvToH264(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    DXGICapture capture;
    X264Encoder encoder(width, height);
    for (int i = 0; i < 100; i++)
    {
        std::cout << i << std::endl;
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size)) {
            if (i == 1) {
                SaveFile("DXGIScreen.rgb", rgbBuffer, rgb32Size);
            }
            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuvBuffer, yuv420Size)) {
                if (i == 1) {
                    SaveFile("DXGIScreen.yuv", yuvBuffer, yuv420Size);
                }

                uint8_t* outData = nullptr;
                size_t size = encoder.EncodeFrame(yuvBuffer, width, height, &outData, i == 0);
                if (size != 0) {
                    SaveFile("DXGIScreen.h264", outData, size);
                }
            }
        }
        Sleep(25);
    }

    delete[] rgbBuffer; rgbBuffer = nullptr;
    delete[] yuvBuffer; yuvBuffer = nullptr;
}

void YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height)
{
    Convert::YUV420ToH264(yuvFilePath, h264FilePath, width, height);
}

void H264ToYUV420(std::string h264FilePath, std::string yuvFilePath, int width, int height)
{
    //Convert::H264ToYUV420Pure(h264FilePath, yuvFilePath, width, height);
    Convert::H264ToYUV420(h264FilePath, yuvFilePath, width, height);
}

void DXGICaptureRgb32ToYuvToH264ToYuv(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuv420Buffer = new unsigned char[yuv420Size];

    unsigned char* decoderYuvBuffer = new unsigned char[yuv420Size]();

    DXGICapture capture;
    X264Encoder encoder(width, height);
    FFmpegDecoder decoder;
    decoder.Init();

    for (int i = 0; i < 100; i++) 
    {
        std::cout << i << std::endl;
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size)) 
        {
            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuv420Buffer, yuv420Size))
            {
                uint8_t* outData = nullptr;
                bool bKeyFrame = i == 0 ? true : false;
                size_t size = encoder.EncodeFrame(yuv420Buffer, width, height, &outData, bKeyFrame);
                if (size != 0) {
                    SaveFile("DXGIScreen.h264", outData, size);
                }
                bool ret = decoder.DecoderFrame(outData, size, decoderYuvBuffer, yuv420Size);
                if (ret) {
                    SaveFile("DXGIScreen.yuv", decoderYuvBuffer, yuv420Size);
                }
            }
        }
    }

    delete[] rgbBuffer; rgbBuffer = nullptr;
    delete[] yuv420Buffer; yuv420Buffer = nullptr;
    delete[] decoderYuvBuffer; decoderYuvBuffer = nullptr;
}

int main()
{
    RECT rect;
    HWND hWnd = GetDesktopWindow();
    GetWindowRect(hWnd, &rect);
    CloseWindow(hWnd);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    //CaptureRgb24(width, height);
    //CaptureRgb32(width, height);

    //DXGICaptureRgb32ToYuvToH264(width, height);

    //YUV420ToH264("DXGIScreen.yuv", "DXGIScreen.h264", width, height);
    //H264ToYUV420("DXGIScreen.h264", "DXGIScreen.yuv", width, height);

    DXGICaptureRgb32ToYuvToH264ToYuv(width, height);

    return 0;
}


/*
使用D3D渲染的代码，需要注掉main函数，并将项目 属性 -> 连接器 -> 系统 -> 子系统 选择 窗口 
*/
//Direct3DRender d3d;
//const int pixel_w = 2240, pixel_h = 1400;
//
//LRESULT WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
//{
//    switch (msg)
//    {
//    case WM_DESTROY:
//        d3d.Cleanup();
//        PostQuitMessage(0);
//        return 0;
//    }
//    return DefWindowProc(hwnd, msg, wparma, lparam);
//}
//
//int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
//{
//    WNDCLASSEX wc;
//    ZeroMemory(&wc, sizeof(wc));
//
//    wc.cbSize = sizeof(wc);
//    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wc.lpfnWndProc = (WNDPROC)MyWndProc;
//    wc.lpszClassName = L"D3D";
//    wc.style = CS_HREDRAW | CS_VREDRAW;
//
//    RegisterClassEx(&wc);
//
//    HWND hwnd = NULL;
//    hwnd = CreateWindow(L"D3D", L"Simplest Video Play Direct3D (Surface)", WS_OVERLAPPEDWINDOW, 100, 100, pixel_w / 2, pixel_h / 2, NULL, NULL, hInstance, NULL);
//    if (hwnd == NULL)
//    {
//        return -1;
//    }
//
//    if (d3d.Init(hwnd, pixel_w, pixel_h) == E_FAIL)
//    {
//        return -1;
//    }
//
//    ShowWindow(hwnd, nShowCmd);
//    UpdateWindow(hwnd);
//
//    FILE* fp = fopen("DXGIScreen.yuv", "rb+");
//    if (fp == NULL)
//    {
//        printf("Cannot open this file.\n");
//        return -1;
//    }
//    unsigned char* buffer = new unsigned char[pixel_w * pixel_h * 3 / 2];
//
//    MSG msg;
//    ZeroMemory(&msg, sizeof(msg));
//
//    while (msg.message != WM_QUIT)
//    {
//        //PeekMessage, not GetMessage
//        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//        else
//        {
//            Sleep(40);
//            fread(buffer, 1, pixel_w * pixel_h * 3 / 2, fp);
//            if (fread(buffer, 1, pixel_w * pixel_h * 3 / 2, fp) != pixel_w * pixel_h * 3 / 2)
//            {
//                fseek(fp, 0, SEEK_SET);//将指针放到头部
//                fread(buffer, 1, pixel_w * pixel_h * 3 / 2, fp);
//            }
//            d3d.Render(buffer);
//        }
//    }
//    delete[] buffer;
//
//    UnregisterClass(L"D3D", hInstance);
//    return 0;
//}
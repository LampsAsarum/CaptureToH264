#include <iostream>
#include <Windows.h>
#include "GDICapture.h"
#include "DXGICapture.h"
#include "Convert.h"
#include "X264Encoder.h"
#include "FFmpegDecoder.h"
#include <vector>

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
    unsigned char* yuvBuffer = (unsigned char*)malloc(width * height * 3 / 2);

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
                bool bKeyFrame = i == 0 ? true : false;
                size_t size = encoder.EncodeFrame(yuvBuffer, width, height, &outData, bKeyFrame);
                if (size != 0) {
                    SaveFile("DXGIScreen.h264", outData, size);
                }
            }
        }
        Sleep(25);
    }
}

void YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height)
{
    Convert::YUV420ToH264(yuvFilePath, h264FilePath, width, height);
}

void H264ToYUV420(std::string h264FilePath, std::string yuvFilePath, int width, int height)
{
    Convert::H264ToYUV420(h264FilePath, yuvFilePath, width, height);
}

void DXGICaptureRgb32ToYuvToH264ToYuv(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    DXGICapture capture;
    X264Encoder encoder(width, height);
    FFmpegDecoder decoder;
    decoder.Init();

    for (int i = 0; i < 10; i++)
    {
        std::cout << i << std::endl;
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size)) 
        {
            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, yuvBuffer, yuv420Size)) 
            {
                uint8_t* outData = nullptr;
                bool bKeyFrame = i == 0 ? true : false;
                size_t size = encoder.EncodeFrame(yuvBuffer, width, height, &outData, bKeyFrame);
                if (size != 0) {
                    SaveFile("DXGIScreen.h264", outData, size);
                }

                decoder.DecoderFrame(outData, size, nullptr, 20);
            }
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

    //CaptureRgb24(width, height);
    //CaptureRgb32(width, height);

    //DXGICaptureRgb32ToYuvToH264(width, height);

    //YUV420ToH264("DXGIScreen.yuv", "DXGIScreen.h264", width, height);
    //H264ToYUV420("DXGIScreen.h264", "DXGIScreen.yuv", width, height);

    DXGICaptureRgb32ToYuvToH264ToYuv(width, height);

    return 0;
}
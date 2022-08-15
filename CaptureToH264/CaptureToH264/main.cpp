#include <iostream>
#include <Windows.h>
#include "GDICapture.h"
#include "DXGICapture.h"
#include "Convert.h"
#include "X264Encoder.h"

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
    if (GDICapture::CaptureRgb24(&rgbBuffer, rgb24Size))
    {
        SaveFile("GDIScreen24.rgb", rgbBuffer, rgb24Size);

        unsigned char* bmpBuffer = nullptr;
        int bmpSize = Convert::Rgb24ToBmp(rgbBuffer, width, height, &bmpBuffer);
        if (bmpSize > 0) {
            SaveFile("GDIScreen24.bmp", bmpBuffer, bmpSize);
        }

        int yuvSize = width * height * 3 / 2;
        unsigned char* yuvBuffer = (unsigned char*)malloc(width * height * 3 / 2);
        if (Convert::Rgb24ToYUV420(rgbBuffer, width, height, yuvBuffer)) {
            SaveFile("GDIScreen24.yuv", yuvBuffer, yuvSize);
        }
    }
}

void CaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

    if (GDICapture::CaptureRgb32(&rgbBuffer, rgb32Size))
    {
        SaveFile("GDIScreen32.rgb", rgbBuffer, rgb32Size);

        unsigned char* bmpBuffer = nullptr;
        int bmpSize = Convert::Rgb32ToBmp(rgbBuffer, width, height, &bmpBuffer);
        if (bmpSize > 0) {
            SaveFile("GDIScreen32.bmp", bmpBuffer, bmpSize);
        }

        if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, &yuvBuffer, yuv420Size)) {
            SaveFile("GDIScreen32.yuv", yuvBuffer, yuv420Size);
        }
    }
}

void DXGICaptureRgb32(int width, int height)
{
    int rgb32Size = width * height * 4;
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];
    int yuv420Size = width * height * 3 / 2;
    unsigned char* yuvBuffer = new unsigned char[yuv420Size];

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

            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, &yuvBuffer, yuv420Size)) {
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
    for (int i = 0; i < 500; i++)
    {
        std::cout << i << std::endl;
        if (capture.CaptureRgb32(&rgbBuffer, rgb32Size)) {
            if (i == 1) {
            SaveFile("DXGIScreen.rgb", rgbBuffer, rgb32Size);
            }
            if (Convert::Rgb32ToYUV420(rgbBuffer, width, height, &yuvBuffer, yuv420Size)) {
                if (i == 1) {
                    SaveFile("DXGIScreen.yuv", yuvBuffer, yuv420Size);
                }

                uint8_t* out_data[8]{ 0 };
                size_t out_linesize[8]{ 0 };
                BYTE* plane[3]{ yuvBuffer, yuvBuffer + width * height, yuvBuffer + width * height * 5 / 4};
                bool bKeyFrame = i == 0 ? true : false;
                encoder.EncodeFrame(plane, out_data, out_linesize, bKeyFrame);
                for (int i = 0; out_data[i]; i++) {
                    SaveFile("DXGIScreen.h264", out_data[i], out_linesize[i]);
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

int main()
{
    RECT rect;
    HWND hWnd = GetDesktopWindow();
    GetWindowRect(hWnd, &rect);
    CloseWindow(hWnd);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    DXGICaptureRgb32ToYuvToH264(width, height);
    //YUV420ToH264("DXGIScreen.yuv", "DXGIScreen.h264", width, height);

    return 0;
}
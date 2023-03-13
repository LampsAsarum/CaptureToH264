#include <iostream>
#include <Windows.h>
#include "ConvertScale.h"
#include "DXGICapture.h"
#include "QsvH264Encoder.h"

void SaveFile(const char* fileName, unsigned char* buf, int size)
{
    FILE* fp = nullptr;
    fopen_s(&fp, fileName, "ab");
    if (!fp) return;
    fwrite(buf, size, 1, fp);
    fclose(fp);
}

void DXGICaptureRgb32(int width, int height)
{
    int rgb32Size = ConvertScale::GetImageSize(width, height, AV_PIX_FMT_BGRA, 16);
    unsigned char* rgbBuffer = new unsigned char[rgb32Size];

    unsigned char* h264Buffer = new unsigned char[rgb32Size];

    DXGICapture capture;
    ConvertScale scale;
    scale.Init(width, height, AV_PIX_FMT_BGRA, width, height, AV_PIX_FMT_NV12);

    QsvH264Encoder encoder;
    encoder.OpenEncoder(width, height, 30, width * height * 3);

    int yuvSize = ConvertScale::GetImageSize(width, height, AV_PIX_FMT_NV12, 16);
    unsigned char* yuvBuffer = new unsigned char[yuvSize];

    char picName[128] = { 0 };

    for (int i = 0; i < 100; i++)
    {
        if (capture.CaptureRgb32(rgbBuffer, rgb32Size))
        {
            if (i > 0)
            {
                /*snprintf(picName, sizeof(picName), "DXGIScreen%d.rgb", i);
                SaveFile(picName, rgbBuffer, rgb32Size);*/
                scale.Convert(rgbBuffer, yuvBuffer);
                /*snprintf(picName, sizeof(picName), "DXGIScreen%d.yuv", i);
                SaveFile(picName, yuvBuffer, yuvSize);*/

                int ret = encoder.Encoder(yuvBuffer, h264Buffer, rgb32Size);
                if (ret > 0)
                {
                    SaveFile("DXGIScreen.h264", h264Buffer, ret);
                    std::cout << ret << std::endl;
                }
            }
        }
        Sleep(100);
    }
    delete[] rgbBuffer; rgbBuffer = nullptr;
}

int main()
{
    bool a = QsvH264Encoder::IsSupported();
    DXGICaptureRgb32(1920, 1080);
    return 0;
}

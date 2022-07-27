#include "Convert.h"
#include <Windows.h>
#include "libyuv/convert_from_argb.h"

namespace
{
    unsigned char ClipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {
        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }
}

/* BMP结构说明
typedef  struct  tagBITMAPFILEHEADER {
    unsigned short int  bfType;       //位图文件的类型，必须为BM
    unsigned long       bfSize;       //文件大小，以字节为单位
    unsigned short int  bfReserverd1; //位图文件保留字，必须为0
    unsigned short int  bfReserverd2; //位图文件保留字，必须为0
    unsigned long       bfbfOffBits;  //位图文件头到数据的偏移量，以字节为单位
}BITMAPFILEHEADER;

typedef  struct  tagBITMAPINFOHEADER
{
    long       biSize;               //该结构大小，字节为单位
    long       biWidth;              //图形宽度以象素为单位
    long       biHeight;             //图形高度以象素为单位
    short int  biPlanes;             //目标设备的级别，必须为1
    short int  biBitcount;           //颜色深度，每个象素所需要的位数
    short int  biCompression;        //位图的压缩类型
    long       biSizeImage;          //位图的大小，以字节为单位
    long       biXPelsPermeter;      //位图水平分辨率，每米像素数
    long       biYPelsPermeter;      //位图垂直分辨率，每米像素数
    long       biClrUsed;            //位图实际使用的颜色表中的颜色数
    long       biClrImportant;       //位图显示过程中重要的颜色数
}BITMAPINFOHEADER;

参考链接：https://www.freesion.com/article/1054806317/
*/

int Convert::Rgb24ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char** bmpBuf)
{
    int rgb24Size = width * height * 3 * sizeof(char);

    BITMAPFILEHEADER bfh;
    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = rgb24Size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0; // reserved  
    bfh.bfReserved2 = 0; // reserved  
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = 0;
    bih.biSizeImage = rgb24Size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb24Size;
    *bmpBuf = new unsigned char[bmpSize];
    memcpy(*bmpBuf, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy(*bmpBuf + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
    memcpy(*bmpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), rgbBuf, rgb24Size);

    return bmpSize;
}

int Convert::Rgb32ToBmp(unsigned char* rgbBuf, int width, int height, unsigned char** bmpBuf)
{
    int rgb32Size = width * height * 4 * sizeof(char);

    BITMAPFILEHEADER bfh;
    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = rgb32Size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0; // reserved  
    bfh.bfReserved2 = 0; // reserved  
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = -height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = 0;
    bih.biSizeImage = rgb32Size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    int bmpSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size;
    *bmpBuf = new unsigned char[bmpSize];
    memcpy(*bmpBuf, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy(*bmpBuf + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
    memcpy(*bmpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), rgbBuf, rgb32Size);

    return bmpSize;
}

int Convert::Rgb24ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char* yuvBuf)
{
    unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
    memset(yuvBuf, 0, width * height * 3 / 2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + width * height;
    ptrV = ptrU + (width * height * 1 / 4);
    unsigned char y, u, v, r, g, b;
    for (int j = 0; j < height; j++)
    {
        ptrRGB = rgbBuf + width * j * 3;
        for (int i = 0; i < width; i++)
        {
            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = ClipValue(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0)
            {
                *(ptrU++) = ClipValue(u, 0, 255);
            }
            else {
                if (i % 2 == 0)
                {
                    *(ptrV++) = ClipValue(v, 0, 255);
                }
            }
        }
    }
    return true;
}

// YUV420p：每4个 Y分量 共用一组 UV分量，前面全是Y分量，后面是U分量，再后面是V分量
int Convert::Rgb32ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char** yuvBuf)
{
    int yuv420Size = width * height * 3 / 2; // Ysize:(width*height); Usize:(width*height)/2; Vsize:(width*height)/2;
    *yuvBuf = new unsigned char[yuv420Size];

    const int yStride = width;
    const int uvStride = (width + 1) / 2;

    const int yLength = width * height;
    const int uvLength = uvStride * ((height + 1) / 2);

    unsigned char* yDataDstPtr = *yuvBuf;
    unsigned char* uDataDstPtr = *yuvBuf + yLength;
    unsigned char* vDataDstPtr = uDataDstPtr + uvLength;

    libyuv::ARGBToI420(rgbBuf, width * 4,// 带转换的argb数据；argb数据每一行的大小
        yDataDstPtr, yStride,            // 用于保存y分量的指针；每一行y分量的长度
        uDataDstPtr, uvStride,           // 用于保存u分量的指针；每一行u分量的长度
        vDataDstPtr, uvStride,           // 用于保存v分量的指针；每一行v分量的长度
        width, height);

    return yuv420Size;
}

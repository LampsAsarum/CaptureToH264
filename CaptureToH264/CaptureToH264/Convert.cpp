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

/* BMP�ṹ˵��
typedef  struct  tagBITMAPFILEHEADER {
    unsigned short int  bfType;       //λͼ�ļ������ͣ�����ΪBM 
    unsigned long       bfSize;       //�ļ���С�����ֽ�Ϊ��λ
    unsigned short int  bfReserverd1; //λͼ�ļ������֣�����Ϊ0 
    unsigned short int  bfReserverd2; //λͼ�ļ������֣�����Ϊ0 
    unsigned long       bfbfOffBits;  //λͼ�ļ�ͷ�����ݵ�ƫ���������ֽ�Ϊ��λ
}BITMAPFILEHEADER;

typedef  struct  tagBITMAPINFOHEADER
{
    long       biSize;               //�ýṹ��С���ֽ�Ϊ��λ
    long       biWidth;              //ͼ�ο��������Ϊ��λ
    long       biHeight;             //ͼ�θ߶�������Ϊ��λ
    short int  biPlanes;             //Ŀ���豸�ļ��𣬱���Ϊ1 
    short int  biBitcount;           //��ɫ��ȣ�ÿ����������Ҫ��λ��
    short int  biCompression;        //λͼ��ѹ������
    long       biSizeImage;          //λͼ�Ĵ�С�����ֽ�Ϊ��λ
    long       biXPelsPermeter;      //λͼˮƽ�ֱ��ʣ�ÿ��������
    long       biYPelsPermeter;      //λͼ��ֱ�ֱ��ʣ�ÿ��������
    long       biClrUsed;            //λͼʵ��ʹ�õ���ɫ���е���ɫ��
    long       biClrImportant;       //λͼ��ʾ��������Ҫ����ɫ��
}BITMAPINFOHEADER;

�ο����ӣ�https://www.freesion.com/article/1054806317/
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

int Convert::Rgb32ToYUV420(unsigned char* rgbBuf, int width, int height, unsigned char** yuvBuf)
{
    int yuv420Size = width * height * 3 / 2;
    *yuvBuf = new unsigned char[yuv420Size];

    //source-stride
    int Dst_Stride_Y = width;
    const int uv_stride = (width + 1) / 2;

    //source-length
    const int y_length = width * height;
    int uv_length = uv_stride * ((height + 1) / 2);

    //source-data
    unsigned char* Y_data_Dst = *yuvBuf;
    unsigned char* U_data_Dst = *yuvBuf + y_length;
    unsigned char* V_data_Dst = U_data_Dst + uv_length;

    //BGRAToI420, �ڴ�˳����BGRA,�����÷����÷�����ARGB
    libyuv::ARGBToI420(rgbBuf,
        width * 4,
        Y_data_Dst, Dst_Stride_Y,
        U_data_Dst, uv_stride,
        V_data_Dst, uv_stride,
        width, height);

    return yuv420Size;
}

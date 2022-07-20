#include "SaveFile.h"
#include <iostream>
#include <Windows.h>

//typedef  struct  tagBITMAPFILEHEADER {
//    unsigned short int  bfType;       //λͼ�ļ������ͣ�����ΪBM 
//    unsigned long       bfSize;       //�ļ���С�����ֽ�Ϊ��λ
//    unsigned short int  bfReserverd1; //λͼ�ļ������֣�����Ϊ0 
//    unsigned short int  bfReserverd2; //λͼ�ļ������֣�����Ϊ0 
//    unsigned long       bfbfOffBits;  //λͼ�ļ�ͷ�����ݵ�ƫ���������ֽ�Ϊ��λ
//}BITMAPFILEHEADER;

//typedef  struct  tagBITMAPINFOHEADER
//{
//    long       biSize;               //�ýṹ��С���ֽ�Ϊ��λ
//    long       biWidth;              //ͼ�ο��������Ϊ��λ
//    long       biHeight;             //ͼ�θ߶�������Ϊ��λ
//    short int  biPlanes;             //Ŀ���豸�ļ��𣬱���Ϊ1 
//    short int  biBitcount;           //��ɫ��ȣ�ÿ����������Ҫ��λ��
//    short int  biCompression;        //λͼ��ѹ������
//    long       biSizeImage;          //λͼ�Ĵ�С�����ֽ�Ϊ��λ
//    long       biXPelsPermeter;      //λͼˮƽ�ֱ��ʣ�ÿ��������
//    long       biYPelsPermeter;      //λͼ��ֱ�ֱ��ʣ�ÿ��������
//    long       biClrUsed;            //λͼʵ��ʹ�õ���ɫ���е���ɫ��
//    long       biClrImportant;       //λͼ��ʾ��������Ҫ����ɫ��
//}BITMAPINFOHEADER;

//https://www.freesion.com/article/1054806317/

void SaveFile::Rgb24(const char* filename, unsigned char* rgbbuf, int width, int height)
{
    int size = width * height * 3 * sizeof(char);

    FILE* fp = nullptr;
    fopen_s(&fp, filename, "wb");
    if (!fp) return;

    fwrite(rgbbuf, size, 1, fp);
    fclose(fp);
}

void SaveFile::Rgb24ToBmp(const char* filename, unsigned char* rgbbuf, int width, int height)
{
    int size = width * height * 3 * sizeof(char);

    BITMAPFILEHEADER bfh;
    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
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
    bih.biSizeImage = size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    FILE* fp = nullptr;
    fopen_s(&fp, filename, "wb");
    if (!fp) return;

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(rgbbuf, size, 1, fp);
    fclose(fp);
}

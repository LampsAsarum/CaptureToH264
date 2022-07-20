#include "SaveFile.h"
#include <iostream>
#include <Windows.h>

//typedef  struct  tagBITMAPFILEHEADER {
//    unsigned short int  bfType;       //位图文件的类型，必须为BM 
//    unsigned long       bfSize;       //文件大小，以字节为单位
//    unsigned short int  bfReserverd1; //位图文件保留字，必须为0 
//    unsigned short int  bfReserverd2; //位图文件保留字，必须为0 
//    unsigned long       bfbfOffBits;  //位图文件头到数据的偏移量，以字节为单位
//}BITMAPFILEHEADER;

//typedef  struct  tagBITMAPINFOHEADER
//{
//    long       biSize;               //该结构大小，字节为单位
//    long       biWidth;              //图形宽度以象素为单位
//    long       biHeight;             //图形高度以象素为单位
//    short int  biPlanes;             //目标设备的级别，必须为1 
//    short int  biBitcount;           //颜色深度，每个象素所需要的位数
//    short int  biCompression;        //位图的压缩类型
//    long       biSizeImage;          //位图的大小，以字节为单位
//    long       biXPelsPermeter;      //位图水平分辨率，每米像素数
//    long       biYPelsPermeter;      //位图垂直分辨率，每米像素数
//    long       biClrUsed;            //位图实际使用的颜色表中的颜色数
//    long       biClrImportant;       //位图显示过程中重要的颜色数
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

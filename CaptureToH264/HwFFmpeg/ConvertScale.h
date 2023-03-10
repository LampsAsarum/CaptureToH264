#pragma once
//ְ��ʹ�� swscale ����ͼ���ת��������

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libswscale/swscale.h"
#include "ffmpeg/libavutil/frame.h"
#include "ffmpeg/libavutil/imgutils.h"
}

class ConvertScale
{
public:
    ConvertScale();
    ~ConvertScale();

    bool Init(int srcWidth, int srcHeight, AVPixelFormat srcPixelFormat,
        int dstWidth, int dstHeight, AVPixelFormat dstPixelFormat);
    bool Destroy();
    bool Convert(unsigned char* srcImage, unsigned char* dstImage);

    static int GetImageSize(int width, int height, AVPixelFormat pixelFormat, int align);

private:
    bool m_bInit;
    SwsContext* m_pConvertContext;

    AVFrame* m_pAVFrameSrc;
    AVFrame* m_pAVFrameDes;

    int m_srcWidth;
    int m_srcHeight;
    AVPixelFormat m_srcPixelFormat;
    int m_dstWidth;
    int m_dstHeight;
    AVPixelFormat m_dstPixelFormat;
};


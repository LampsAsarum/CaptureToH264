#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
};

class FFmpegDecoder
{
public:
    FFmpegDecoder();
    ~FFmpegDecoder();

    bool Init();
    bool DecoderFrame(unsigned char* cur_ptr, int cur_size, unsigned char* outYuvBuffer, const int yuv420Size);

private:
    AVFrame* pFrame; //�洢һ֡��������������
    AVPacket* packet; // �洢һ֡��һ������£�ѹ����������

    AVCodecContext* pCodecCtx;
    AVCodecParserContext* pCodecParserCtx;
};


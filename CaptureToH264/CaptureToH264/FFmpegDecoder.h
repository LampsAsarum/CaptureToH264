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
    AVFrame* pFrame; //存储一帧解码后的像素数据
    AVPacket* packet; // 存储一帧（一般情况下）压缩编码数据

    AVCodecContext* pCodecCtx;
    AVCodecParserContext* pCodecParserCtx;
};


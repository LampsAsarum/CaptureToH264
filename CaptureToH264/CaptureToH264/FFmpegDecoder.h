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
    bool DecoderFrame(unsigned char* cur_ptr, int cur_size, unsigned char* out, const int outSize);

private:
    AVCodecContext* pCodecCtx;
    AVCodecParserContext* pCodecParserCtx;
};


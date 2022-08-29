#pragma once

#include <stdint.h>

extern "C"
{
#include "x264.h"
};

class X264Encoder
{
public:
    X264Encoder(int width, int height);
    ~X264Encoder();

    bool EncodeFrame(
        uint8_t* in_yuvbuf, const int width, const int height,
        uint8_t* out_ppData[8],
        size_t   out_linesize[8],
        bool in_forceKey);

private:
    bool Encode(
        x264_picture_t* pic_in,
        uint8_t* out_ppData[8],
        size_t   out_linesize[8]);

private:
    x264_t* m_pHandle;
    x264_picture_t m_pic_in;
};


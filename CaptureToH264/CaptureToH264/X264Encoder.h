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

    size_t EncodeFrame(uint8_t* inYuvbuf, const int inWidth, const int inHeight, uint8_t** out_ppData, bool in_forceKey);

private:
    size_t Encode(x264_picture_t* pic_in, uint8_t** out_ppData);

private:
    x264_t* m_pX264;
    x264_picture_t m_pic_in;
};


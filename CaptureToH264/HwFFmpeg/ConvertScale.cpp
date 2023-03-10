#include "ConvertScale.h"
#pragma warning(disable:4996)

ConvertScale::ConvertScale() 
    : m_bInit(true)
    , m_pConvertContext(nullptr)
    , m_pAVFrameSrc(nullptr)
    , m_pAVFrameDes(nullptr)
    , m_srcWidth(0)
    , m_srcHeight(0)
    , m_srcPixelFormat(AV_PIX_FMT_NONE)
    , m_dstWidth(0)
    , m_dstHeight(0)
    , m_dstPixelFormat(AV_PIX_FMT_NONE)
{
}

ConvertScale::~ConvertScale()
{
    Destroy();
}

bool ConvertScale::Init(int srcWidth, int srcHeight, AVPixelFormat srcPixelFormat, int dstWidth, int dstHeight, AVPixelFormat dstPixelFormat)
{
    m_pAVFrameSrc = av_frame_alloc();
    if (!m_pAVFrameSrc)
    {
        return false;
    }
    m_pAVFrameDes = av_frame_alloc();
    if (!m_pAVFrameSrc)
    {
        return false;
    }
    m_pConvertContext = sws_getContext(srcWidth, srcHeight, srcPixelFormat, dstWidth, dstHeight, dstPixelFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (!m_pConvertContext)
    {
        return false;
    }
    m_bInit = true;
    m_srcWidth = srcWidth;
    m_srcHeight = srcHeight;
    m_srcPixelFormat = srcPixelFormat;
    m_dstWidth = dstWidth;
    m_dstHeight = dstHeight;
    m_dstPixelFormat = dstPixelFormat;
    return true;
}

bool ConvertScale::Destroy()
{
    if (m_pConvertContext)
    {
        sws_freeContext(m_pConvertContext);
        m_pConvertContext = nullptr;
    }
    if (m_pAVFrameSrc)
    {
        av_frame_free(&m_pAVFrameSrc);
        m_pAVFrameSrc = nullptr;
    }
    if (m_pAVFrameDes)
    {
        av_frame_free(&m_pAVFrameDes);
        m_pAVFrameDes = nullptr;
    }
    m_bInit = false;
    return false;
}

bool ConvertScale::Convert(unsigned char* srcImage, unsigned char* dstImage)
{
    avpicture_fill((AVPicture*)m_pAVFrameSrc, srcImage, m_srcPixelFormat, m_srcWidth, m_srcHeight);
    avpicture_fill((AVPicture*)m_pAVFrameDes, dstImage, m_dstPixelFormat, m_dstWidth, m_dstHeight);

    int rDesHeight = sws_scale(m_pConvertContext, (const uint8_t* const*)m_pAVFrameSrc->data, m_pAVFrameSrc->linesize,
        0, m_srcHeight, m_pAVFrameDes->data, m_pAVFrameDes->linesize);
    if (rDesHeight <= 0)
    {
        return false;
    }
    return true;
}

int ConvertScale::GetImageSize(int width, int height, AVPixelFormat pixelFormat, int align)
{
    return av_image_get_buffer_size(pixelFormat, width, height, align);
}

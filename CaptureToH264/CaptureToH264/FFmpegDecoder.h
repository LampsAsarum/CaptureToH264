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

    /// <summary>
    /// 将 h264 数据解码为 YUV 数据
    /// </summary>
    /// <param name="h264Buffer">h264输入数据</param>
    /// <param name="h264Size">输入数据大小</param>
    /// <param name="outYuvBuffer">输出YUV数据</param>
    /// <returns>输出YUV数据大小</returns>
    /// <note>现版本是在该函数内申请空间，如果解出多帧，还会自动扩容，但因为牵扯到内存申请与拷贝，所以效率不高，后续优化</note>
    unsigned int DecoderFrame(unsigned char* h264Buffer, int h264Size, unsigned char** outYuvBuffer);

private:
    AVFrame* pFrame; //存储一帧解码后的像素数据
    AVPacket* packet; // 存储一帧（一般情况下）压缩编码数据

    AVCodecContext* pCodecCtx;
    AVCodecParserContext* pCodecParserCtx;
};


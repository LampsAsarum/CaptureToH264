#pragma once

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavcodec/qsv.h"
#include "ffmpeg/libavutil/imgutils.h"
#include "ffmpeg/libavutil/opt.h"
#include "ffmpeg/libavutil/hwcontext.h"
#include "ffmpeg/libswscale/swscale.h"
}

class QsvH264Encoder
{
public:
    QsvH264Encoder();
    ~QsvH264Encoder();

    static bool IsSupported();

    /// <summary>
    /// 初始化编码器
    /// </summary>
    /// <param name="width">h264的宽</param>
    /// <param name="height">h264的高</param>
    /// <param name="frameRate">帧率</param>
    /// <param name="bitrate">码率</param>
    /// <returns></returns>
    bool OpenEncoder(int width, int height, int frameRate, int bitrate);
    void CloseEncoder();

    /// <summary>
    /// 编码一帧
    /// </summary>
    /// <param name="inImageData">编码一帧的数据</param>
    /// <param name="h264Buffer">存放H264数据的Buffer</param>
    /// <param name="h264BufferSize">存放H264数据的Buffer的大小</param>
    /// <returns>编码后的数据大小</returns>
    int Encoder(const unsigned char* inImageData, unsigned char* h264Buffer, int h264BufferSize);

private:
    AVCodecContext* m_pEncoderContext = nullptr;

    AVFrame* m_pAVFrame = nullptr;
};


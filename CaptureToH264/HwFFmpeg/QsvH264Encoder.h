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
    /// ��ʼ��������
    /// </summary>
    /// <param name="width">h264�Ŀ�</param>
    /// <param name="height">h264�ĸ�</param>
    /// <param name="frameRate">֡��</param>
    /// <param name="bitrate">����</param>
    /// <returns></returns>
    bool OpenEncoder(int width, int height, int frameRate, int bitrate);
    void CloseEncoder();

    /// <summary>
    /// ����һ֡
    /// </summary>
    /// <param name="inImageData">����һ֡������</param>
    /// <param name="h264Buffer">���H264���ݵ�Buffer</param>
    /// <param name="h264BufferSize">���H264���ݵ�Buffer�Ĵ�С</param>
    /// <returns>���������ݴ�С</returns>
    int Encoder(const unsigned char* inImageData, unsigned char* h264Buffer, int h264BufferSize);

private:
    AVCodecContext* m_pEncoderContext = nullptr;

    AVFrame* m_pAVFrame = nullptr;
};


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
    /// �� h264 ���ݽ���Ϊ YUV ����
    /// </summary>
    /// <param name="h264Buffer">h264��������</param>
    /// <param name="h264Size">�������ݴ�С</param>
    /// <param name="outYuvBuffer">���YUV����</param>
    /// <returns>���YUV���ݴ�С</returns>
    /// <note>�ְ汾���ڸú���������ռ䣬��������֡�������Զ����ݣ�����Ϊǣ�����ڴ������뿽��������Ч�ʲ��ߣ������Ż�</note>
    unsigned int DecoderFrame(unsigned char* h264Buffer, int h264Size, unsigned char** outYuvBuffer);

private:
    AVFrame* pFrame; //�洢һ֡��������������
    AVPacket* packet; // �洢һ֡��һ������£�ѹ����������

    AVCodecContext* pCodecCtx;
    AVCodecParserContext* pCodecParserCtx;
};


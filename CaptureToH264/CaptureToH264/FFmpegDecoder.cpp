#include "FFmpegDecoder.h"

#pragma warning(disable:4996)


FFmpegDecoder::FFmpegDecoder()
{
    pFrame = av_frame_alloc();
    packet = av_packet_alloc();

    pCodecParserCtx = nullptr;
    pCodecCtx = nullptr;
}

FFmpegDecoder::~FFmpegDecoder()
{
    av_parser_close(pCodecParserCtx);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);

    av_frame_free(&pFrame);
}

bool FFmpegDecoder::Init()
{
    avcodec_register_all(); //注册所有的编解码器。

    AVCodec* pCodec = avcodec_find_decoder(AV_CODEC_ID_H264); //查找解码器
    if (!pCodec) {
        printf("Codec not found\n");
        return false;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec); //为AVCodecContext分配内存
    if (!pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264); //初始化AVCodecParserContext
    if (!pCodecParserCtx) {
        printf("Could not allocate video parser context\n");
        return false;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //打开解码器
        printf("Could not open codec\n");
        return false;
    }

    return true;
}

unsigned int FFmpegDecoder::DecoderFrame(unsigned char* h264Buffer, int h264Size, unsigned char** outYuvBuffer)
{
    int lastSize = 0;
    int currentSize = 0;

    while (h264Size > 0)
    {
        /*  av_parser_parse2 使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据。 解析获得一个Packet
        int av_parser_parse2(AVCodecParserContext *s, AVCodecContext* avctx,
                            uint8_t** poutbuf, int* poutbuf_size,
                            const uint8_t* buf, int buf_size,
                            int64_t pts, int64_t dts, int64_t pos);
        其中poutbuf指向解析后输出的压缩编码数据帧，buf指向输入的压缩编码数据。如果函数执行完后输出数据为空（poutbuf_size为0），
        则代表解析还没有完成，还需要再次调用av_parser_parse2()解析一部分数据才可以得到解析后的数据帧。
        当函数执行完后输出数据不为空的时候，代表解析完成，可以将poutbuf中的这帧数据取出来做后续处理。*/
        int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,
            &packet->data, &packet->size, h264Buffer, h264Size,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

        h264Buffer += len;
        h264Size -= len;

        if (packet->size == 0)
            continue;

        printf("[Packet]Size:%6d\t", packet->size);
        switch (pCodecParserCtx->pict_type) {
        case AV_PICTURE_TYPE_I: printf("Type:I\t"); break;
        case AV_PICTURE_TYPE_P: printf("Type:P\t"); break;
        case AV_PICTURE_TYPE_B: printf("Type:B\t"); break;
        default: printf("Type:Other\t"); break;
        }
        printf("Number:%4d\n", pCodecParserCtx->output_picture_number);

        int got_picture;
        if (avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet) < 0) {
            printf("Decode Error.\n");
            return 0;
        }

        if (got_picture)
        {
            // 自动扩容
            lastSize = currentSize;
            currentSize += pFrame->width * pFrame->height * 3 / 2;

            unsigned char* tmp = new unsigned char[currentSize]();
            memcpy(tmp, *outYuvBuffer, lastSize);
            delete[](*outYuvBuffer);
            *outYuvBuffer = tmp;
            unsigned char* yuvBuffer = (*outYuvBuffer) + lastSize;

            for (int i = 0; i < pFrame->height; i++) {
                memcpy(yuvBuffer + pFrame->width * i,
                    pFrame->data[0] + pFrame->linesize[0] * i,
                    pFrame->width);
            }

            for (int i = 0; i < pFrame->height / 2; i++) {
                memcpy(yuvBuffer + (pFrame->width * pFrame->height) + (pFrame->width / 2) * i,
                    pFrame->data[1] + pFrame->linesize[1] * i,
                    pFrame->width / 2);
            }

            for (int i = 0; i < pFrame->height / 2; i++) {
                memcpy(yuvBuffer + pFrame->width * pFrame->height + ((pFrame->width / 2) * (pFrame->height / 2)) + pFrame->width / 2 * i,
                    pFrame->data[2] + pFrame->linesize[2] * i,
                    pFrame->width / 2);
            }
            printf("Succeed to decode 1 frame!\n");

        }
    }

    return currentSize;
}
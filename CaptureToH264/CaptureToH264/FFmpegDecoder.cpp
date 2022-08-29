#include "FFmpegDecoder.h"

#pragma warning(disable:4996)


FFmpegDecoder::FFmpegDecoder()
{

}

FFmpegDecoder::~FFmpegDecoder()
{
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

static int length = 0;
bool FFmpegDecoder::DecoderFrame(unsigned char* cur_ptr, int cur_size, unsigned char* out, const int outSize)
{
    AVFrame* pFrame = av_frame_alloc(); //存储一帧解码后的像素数据
    AVPacket* packet = av_packet_alloc(); // 存储一帧（一般情况下）压缩编码数据

    do {
        /*  av_parser_parse2 使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据。 解析获得一个Packet
        int av_parser_parse2(AVCodecParserContext *s, AVCodecContext* avctx,
                            uint8_t** poutbuf, int* poutbuf_size,
                            const uint8_t* buf, int buf_size,
                            int64_t pts, int64_t dts, int64_t pos);
        其中poutbuf指向解析后输出的压缩编码数据帧，buf指向输入的压缩编码数据。如果函数执行完后输出数据为空（poutbuf_size为0），
        则代表解析还没有完成，还需要再次调用av_parser_parse2()解析一部分数据才可以得到解析后的数据帧。
        当函数执行完后输出数据不为空的时候，代表解析完成，可以将poutbuf中的这帧数据取出来做后续处理。*/
        int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,
            &packet->data, &packet->size, cur_ptr, cur_size,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        cur_ptr += len;
        cur_size -= len;
    } while (packet->size == 0);


    //Some Info from AVCodecParserContext
    printf("[Packet]Size:%d\n", packet->size); length += packet->size;
    switch (pCodecParserCtx->pict_type) {
    case AV_PICTURE_TYPE_I: printf("Type:I\n"); break;
    case AV_PICTURE_TYPE_P: printf("Type:P\n"); break;
    case AV_PICTURE_TYPE_B: printf("Type:B\n"); break;
    default: printf("Type:Other\n"); break;
    }
    //printf("Number:%4d\n", pCodecParserCtx->output_picture_number);

    int got_picture;
    int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //解码一帧数据
    if (ret < 0) {
        printf("Decode Error.\n");
        return false;
    }

    FILE* fp_out = fopen("text.yuv", "ab");
    if (got_picture)
    {
        //Y, U, V
        for (int i = 0; i < pFrame->height; i++) {
            fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
        }
        for (int i = 0; i < pFrame->height / 2; i++) {
            fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
        }
        for (int i = 0; i < pFrame->height / 2; i++) {
            fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
        }
    }

    printf("[ALL]length:%d\n", length);
}

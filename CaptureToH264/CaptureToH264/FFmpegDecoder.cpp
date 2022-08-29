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

bool FFmpegDecoder::DecoderFrame(unsigned char* cur_ptr, int cur_size, unsigned char* out, const int outSize)
{
    AVFrame* pFrame = av_frame_alloc(); //存储一帧解码后的像素数据
    AVPacket* packet = av_packet_alloc(); // 存储一帧（一般情况下）压缩编码数据

    /*const int in_buffer_size = 4096;
    unsigned char in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };*/
    int got_picture;

    int first_time = 1;

    while (cur_size > 0)
    {
        //使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据。 解析获得一个Packet
        int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,
            &packet->data, &packet->size, cur_ptr, cur_size,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

        cur_ptr += len;
        cur_size -= len;

        if (packet->size == 0)
            continue;

        //Some Info from AVCodecParserContext
        printf("[Packet]Size:%6d\t", packet->size);
        switch (pCodecParserCtx->pict_type) {
        case AV_PICTURE_TYPE_I: printf("Type:I\t"); break;
        case AV_PICTURE_TYPE_P: printf("Type:P\t"); break;
        case AV_PICTURE_TYPE_B: printf("Type:B\t"); break;
        default: printf("Type:Other\t"); break;
        }
        printf("Number:%4d\n", pCodecParserCtx->output_picture_number);




        int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //解码一帧数据
        if (ret < 0) {
            printf("Decode Error.\n");
            return false;
        }

        FILE* fp_out = fopen("text.yuv", "ab");
        if (got_picture)
        {
            if (first_time) {
                printf("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
                printf("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
                first_time = 0;
            }
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

            printf("Succeed to decode 1 frame!\n");
        }
    }
}

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
    avcodec_register_all(); //ע�����еı��������

    AVCodec* pCodec = avcodec_find_decoder(AV_CODEC_ID_H264); //���ҽ�����
    if (!pCodec) {
        printf("Codec not found\n");
        return false;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec); //ΪAVCodecContext�����ڴ�
    if (!pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264); //��ʼ��AVCodecParserContext
    if (!pCodecParserCtx) {
        printf("Could not allocate video parser context\n");
        return false;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //�򿪽�����
        printf("Could not open codec\n");
        return false;
    }

    return true;
}

bool FFmpegDecoder::DecoderFrame(unsigned char* cur_ptr, int cur_size, unsigned char* outYuvBuffer, const int yuv420Size)
{
    bool ret = false;

    //FILE* fp_out = fopen("text.yuv", "ab");
    while (cur_size > 0)
    {
        /*  av_parser_parse2 ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ���������ݡ� �������һ��Packet
        int av_parser_parse2(AVCodecParserContext *s, AVCodecContext* avctx,
                            uint8_t** poutbuf, int* poutbuf_size,
                            const uint8_t* buf, int buf_size,
                            int64_t pts, int64_t dts, int64_t pos);
        ����poutbufָ������������ѹ����������֡��bufָ�������ѹ���������ݡ��������ִ������������Ϊ�գ�poutbuf_sizeΪ0����
        ����������û����ɣ�����Ҫ�ٴε���av_parser_parse2()����һ�������ݲſ��Եõ������������֡��
        ������ִ�����������ݲ�Ϊ�յ�ʱ�򣬴��������ɣ����Խ�poutbuf�е���֡����ȡ��������������*/
        int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,
            &packet->data, &packet->size, cur_ptr, cur_size,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        cur_ptr += len;
        cur_size -= len;

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
            return false;
        }
        if (got_picture) {
            //Y, U, V
            for (int i = 0; i < pFrame->height; i++) {
                //fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
                memcpy_s(outYuvBuffer + pFrame->linesize[0] * i, yuv420Size, 
                    pFrame->data[0] + pFrame->linesize[0] * i, pFrame->width);
            }
            for (int i = 0; i < pFrame->height / 2; i++) {
               // fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
                memcpy_s(outYuvBuffer + pFrame->width * pFrame->height + pFrame->linesize[1] * i, yuv420Size, 
                    pFrame->data[1] + pFrame->linesize[1] * i, pFrame->width / 2);
            }
            for (int i = 0; i < pFrame->height / 2; i++) {
                //fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
                memcpy_s(outYuvBuffer + pFrame->width * pFrame->height + (pFrame->width * pFrame->height) / 4 + pFrame->linesize[2] * i, yuv420Size,
                    pFrame->data[2] + pFrame->linesize[2] * i, pFrame->width / 2);
            }
            printf("Succeed to decode 1 frame!\n");
            ret = true;
        }
    }

    return ret;

    //Flush Decoder
    //packet->data = NULL;
    //packet->size = 0;
    //while (true) {
    //    int got_picture;
    //    int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
    //    if (ret < 0) {
    //        printf("Decode Error.\n");
    //        return ret;
    //    }
    //    if (!got_picture) {
    //        break;
    //    }
    //    else {
    //        //Y, U, V
    //        for (int i = 0; i < pFrame->height; i++) {
    //            fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
    //        }
    //        for (int i = 0; i < pFrame->height / 2; i++) {
    //            fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
    //        }
    //        for (int i = 0; i < pFrame->height / 2; i++) {
    //            fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
    //        }
    //        printf("Flush Decoder: Succeed to decode 1 frame!\n");
    //    }
    //}

    //fclose(fp_out);
}

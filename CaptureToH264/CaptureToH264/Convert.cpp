#include "Convert.h"
#include <Windows.h>
#include "libyuv/convert_from_argb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern "C"
{
#include "x264.h"
#include "libavcodec/avcodec.h"

#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
};

#pragma warning(disable:4996)

namespace
{
    unsigned char ClipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {
        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }
}

/* BMP�ṹ˵��
typedef  struct  tagBITMAPFILEHEADER {
    unsigned short int  bfType;       //λͼ�ļ������ͣ�����ΪBM
    unsigned long       bfSize;       //�ļ���С�����ֽ�Ϊ��λ
    unsigned short int  bfReserverd1; //λͼ�ļ������֣�����Ϊ0
    unsigned short int  bfReserverd2; //λͼ�ļ������֣�����Ϊ0
    unsigned long       bfbfOffBits;  //λͼ�ļ�ͷ�����ݵ�ƫ���������ֽ�Ϊ��λ
}BITMAPFILEHEADER;

typedef  struct  tagBITMAPINFOHEADER
{
    long       biSize;               //�ýṹ��С���ֽ�Ϊ��λ
    long       biWidth;              //ͼ�ο��������Ϊ��λ
    long       biHeight;             //ͼ�θ߶�������Ϊ��λ
    short int  biPlanes;             //Ŀ���豸�ļ��𣬱���Ϊ1
    short int  biBitcount;           //��ɫ��ȣ�ÿ����������Ҫ��λ��
    short int  biCompression;        //λͼ��ѹ������
    long       biSizeImage;          //λͼ�Ĵ�С�����ֽ�Ϊ��λ
    long       biXPelsPermeter;      //λͼˮƽ�ֱ��ʣ�ÿ��������
    long       biYPelsPermeter;      //λͼ��ֱ�ֱ��ʣ�ÿ��������
    long       biClrUsed;            //λͼʵ��ʹ�õ���ɫ���е���ɫ��
    long       biClrImportant;       //λͼ��ʾ��������Ҫ����ɫ��
}BITMAPINFOHEADER;

�ο����ӣ�https://www.freesion.com/article/1054806317/
*/

bool Convert::Rgb24ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize)
{
    int rgb24Size = width * height * 3;
    if (bmpBufferSize < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb24Size) {
        return false;
    }

    BITMAPFILEHEADER bfh;
    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = rgb24Size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0; // reserved  
    bfh.bfReserved2 = 0; // reserved  
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = 0;
    bih.biSizeImage = rgb24Size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    memset(bmpBuf, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb24Size);

    memcpy(bmpBuf, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy(bmpBuf + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
    memcpy(bmpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), rgbBuf, rgb24Size);

    return true;
}

bool Convert::Rgb32ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize)
{
    int rgb32Size = width * height * 4;
    if (bmpBufferSize < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size) {
        return false;
    }

    BITMAPFILEHEADER bfh;
    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = rgb32Size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0; // reserved  
    bfh.bfReserved2 = 0; // reserved  
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = -height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = 0;
    bih.biSizeImage = rgb32Size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    memset(bmpBuf, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rgb32Size);

    memcpy(bmpBuf, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy(bmpBuf + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
    memcpy(bmpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), rgbBuf, rgb32Size);

    return true;
}

bool Convert::Rgb24ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize)
{
    if (yuvBufferSize < width * height * 3 / 2) {
        return false;
    }

    memset(yuvBuf, 0, width * height * 3 / 2);
    unsigned char* ptrY = yuvBuf;
    unsigned char* ptrU = yuvBuf + width * height;
    unsigned char* ptrV = ptrU + (width * height * 1 / 4);

    unsigned char y, u, v, r, g, b;
    unsigned char* ptrRGB;
    for (int j = 0; j < height; j++)
    {
        ptrRGB = rgbBuf + width * j * 3;
        for (int i = 0; i < width; i++)
        {
            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = ClipValue(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0)
            {
                *(ptrU++) = ClipValue(u, 0, 255);
            }
            else {
                if (i % 2 == 0)
                {
                    *(ptrV++) = ClipValue(v, 0, 255);
                }
            }
        }
    }
    return true;
}

// YUV420p��ÿ4�� Y���� ����һ�� UV������ǰ��ȫ��Y������������U�������ٺ�����V����
bool Convert::Rgb32ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize)
{
    // Ysize:(width*height); Usize:(width*height)/2; Vsize:(width*height)/2;
    if (yuvBufferSize < width * height * 3 / 2) {
        return false;
    }

    memset(yuvBuf, 0, width * height * 3 / 2);

    const int yStride = width;
    const int uvStride = (width + 1) / 2;

    const int yLength = width * height;
    const int uvLength = uvStride * ((height + 1) / 2);

    unsigned char* yDataDstPtr = yuvBuf;
    unsigned char* uDataDstPtr = yuvBuf + yLength;
    unsigned char* vDataDstPtr = uDataDstPtr + uvLength;

    libyuv::ARGBToI420(rgbBuf, width * 4,// ��ת����argb���ݣ�argb����ÿһ�еĴ�С
        yDataDstPtr, yStride,            // ���ڱ���y������ָ�룻ÿһ��y�����ĳ���
        uDataDstPtr, uvStride,           // ���ڱ���u������ָ�룻ÿһ��u�����ĳ���
        vDataDstPtr, uvStride,           // ���ڱ���v������ָ�룻ÿһ��v�����ĳ���
        width, height);

    return true;
}

bool Convert::YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height)
{
    FILE* fp_in = fopen(yuvFilePath.c_str(), "rb");
    FILE* fp_out = fopen(h264FilePath.c_str(), "wb");
    if (fp_in == NULL || fp_out == NULL) {
        printf("Error open files.\n");
        return false;
    }

    int csp = X264_CSP_I420;

    x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
    x264_param_default(pParam); //���ò������ṹ��x264_param_t��ȱʡֵ
    pParam->i_width = width;                          // ��Ƶͼ��Ŀ�
    pParam->i_height = height;                        // ��Ƶͼ��ĸ�
    pParam->i_csp = csp;                              // �����������CSP
    pParam->i_frame_total = 0;                        // ����֡������, Ĭ�� 0
    pParam->i_log_level = X264_LOG_DEBUG;             // ��־���𣬲���Ҫ��ӡ������Ϣʱֱ��ע�͵�����
    pParam->i_threads = X264_SYNC_LOOKAHEAD_AUTO;     // ���б����֡; �߳�����Ϊ0���Զ����̱߳���
    pParam->i_keyint_max = 10;                        // �趨IDR֮֡����������ڴ˼������IDR�ؼ�֡
    pParam->i_bframe = 5;                             // �����ο�֮֡���B֡��Ŀ
    pParam->b_open_gop = 0;                           // Close GOP��ָ֡���Ԥ�ⶼ����GOP�н��еġ�
    pParam->i_bframe_pyramid = 0;                     // ������B֡Ϊ�ο�֡��
    pParam->rc.i_qp_constant = 0;                     // ָ��P֡������ֵ��0 - 51��0��ʾ����
    pParam->rc.i_qp_max = 0;                          // ������������ֵ��Ĭ��51
    pParam->rc.i_qp_min = 0;                          // �������С����ֵ��Ĭ��10
    pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS; // ����ӦB֡�ж�, ��ѡȡֵ��X264_B_ADAPT_FAST��
    pParam->i_fps_num = 25;                           // ֡�ʵķ���
    pParam->i_fps_den = 1;                            // ֡�ʵķ�ĸ
    pParam->i_timebase_num = pParam->i_fps_den;       // ʱ����ķ���
    pParam->i_timebase_den = pParam->i_fps_num;       // ʱ����ķ�ĸ
    x264_param_apply_profile(pParam, x264_profile_names[1]); // ���ñ����profile, ֧�ֵ�profile��x264_profile_names��

    x264_t* pHandle = x264_encoder_open(pParam);//�򿪱�����
    x264_nal_t* pNals = NULL;//�洢ѹ����������������
    x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));// �洢ѹ������ǰ����������
    x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));

    x264_picture_init(pPic_out);//�������Ӧ�ó�������Լ���x264_picture_t������ʹ��x264_picture_alloc������Ҫִ�д˲���
    x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);//Ϊͼ��ṹ��x264_picture_t�����ڴ�

    int iNal = 0;
    int ret = x264_encoder_headers(pHandle, &pNals, &iNal);//�������SPS/PPS/SEI��ЩH.264������ͷ��Ϣ

    int y_size = pParam->i_width * pParam->i_height;

    int frame_num = 0;
    fseek(fp_in, 0, SEEK_END);
    switch (csp)
    {
    case X264_CSP_I444:
        frame_num = ftell(fp_in) / (y_size * 3);
        break;
    case X264_CSP_I420: {
        int a = ftell(fp_in);
        int b = (y_size * 3 / 2);
        frame_num = ftell(fp_in) / (y_size * 3 / 2);
        break;
    }
    default:
        printf("Colorspace Not Support.\n");
        return false;
    }
    fseek(fp_in, 0, SEEK_SET);

    //Loop to Encode
    for (int i = 0; i < frame_num; i++)
    {
        switch (csp)
        {
        case X264_CSP_I444:
            fread(pPic_in->img.plane[0], y_size, 1, fp_in);	//Y
            fread(pPic_in->img.plane[1], y_size, 1, fp_in);	//U
            fread(pPic_in->img.plane[2], y_size, 1, fp_in);	//V
            break;
        case X264_CSP_I420:
            fread(pPic_in->img.plane[0], y_size, 1, fp_in);	//Y
            fread(pPic_in->img.plane[1], y_size / 4, 1, fp_in);	//U
            fread(pPic_in->img.plane[2], y_size / 4, 1, fp_in);	//V
            break;
        default:
            printf("Colorspace Not Support.\n");
            return false;
        }
        pPic_in->i_pts = i;

        ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);//����һ֡ͼ��
        if (ret < 0) {
            printf("Error.\n");
            return false;
        }

        printf("Succeed encode frame: %5d\n", i);

        for (int j = 0; j < iNal; ++j) {
            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_out);
        }
    }

    int i = 0;
    //flush encoder �����������ʣ�����������
    while (1)
    {
        ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);//����һ֡ͼ��
        if (ret == 0) {
            break;
        }
        printf("Flush 1 frame.\n");
        for (int j = 0; j < iNal; ++j) {
            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_out);
        }
        i++;
    }

    x264_picture_clean(pPic_in);//�ͷ�x264_picture_alloc()�������Դ
    x264_encoder_close(pHandle);//�رձ�����
    pHandle = NULL;

    free(pPic_in);
    free(pPic_out);
    free(pParam);

    fclose(fp_in);
    fclose(fp_out);

    return true;
}

bool Convert::H264ToYUV420Pure(std::string h264FilePath, std::string yuvFilePath, int width, int height)
{
    FILE* fp_in = fopen(h264FilePath.c_str(), "rb");
    FILE* fp_out = fopen(yuvFilePath.c_str(), "wb");
    if (fp_in == NULL || fp_out == NULL) {
        printf("Error open files.\n");
        return false;
    }

    avcodec_register_all(); //ע�����еı��������
    //ֻע���������йص����������˵�����������������������˾��ȣ����ǲ�ע�Ḵ��/�⸴������Щ�ͱ�������޹ص������

    AVCodec* pCodec = avcodec_find_decoder(AV_CODEC_ID_H264); //���ҽ�����
    if (!pCodec) {
        printf("Codec not found\n");
        return false;
    }
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec); //ΪAVCodecContext�����ڴ�
    if (!pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    AVCodecParserContext* pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264); //��ʼ��AVCodecParserContext
    if (!pCodecParserCtx) {
        printf("Could not allocate video parser context\n");
        return false;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //�򿪽�����
        printf("Could not open codec\n");
        return false;
    }


    AVFrame* pFrame = av_frame_alloc(); //�洢һ֡��������������
    AVPacket* packet = av_packet_alloc(); // �洢һ֡��һ������£�ѹ����������
    //AVPacket packet; ���ֳ�ʼ����ʽ��������
    //av_init_packet(&packet);

    const int in_buffer_size = 4096;
    unsigned char in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };
    unsigned char* cur_ptr;
    int cur_size;
    int ret, got_picture;

    int first_time = 1;
    while (true)
    {
        cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
        if (cur_size == 0)
            break;
        cur_ptr = in_buffer;

        while (cur_size > 0)
        {
            //ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ���������ݡ�
            int len = av_parser_parse2( // �������һ��Packet
                pCodecParserCtx, pCodecCtx,
                &packet->data, &packet->size,
                cur_ptr, cur_size,
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

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //����һ֡����
            if (ret < 0) {
                printf("Decode Error.\n");
                return false;
            }

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

    //Flush Decoder
    packet->data = NULL;
    packet->size = 0;
    while (1)
    {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //����һ֡����
        if (ret < 0) {
            printf("Decode Error.\n");
            return false;
        }
        if (!got_picture) {
            break;
        }
        else {
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

            printf("Flush Decoder: Succeed to decode 1 frame!\n");
        }
    }

    fclose(fp_in);
    fclose(fp_out);

    av_parser_close(pCodecParserCtx);

    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);

    return true;
}

bool Convert::H264ToYUV420(std::string h264FilePath, std::string yuvFilePath, int width, int height)
{
    FILE* fp_out = fopen(yuvFilePath.c_str(), "wb+");
    if (fp_out == NULL) {
        printf("Error open files.\n");
        return false;
    }

    av_register_all();//ע���˺ͱ�������йص����
    AVFormatContext* pFormatCtx = avformat_alloc_context();//����AVFormatContext�ṹ��

    //1. ��ý���ļ�
    if (avformat_open_input(&pFormatCtx, h264FilePath.c_str(), NULL, NULL) != 0) { //����Ƶ��
        printf("Couldn't open input stream.\n");
        return false;
    }
    //2.��ȡý���ļ���Ϣ����avFormatContext��ֵ
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) { //��ȡ����Ϣ
        printf("Couldn't find stream information.\n");
        return false;
    }
    av_dump_format(pFormatCtx, 0, h264FilePath.c_str(), 0); // ��ӡ����Ϣ

    //3. ƥ�䵽��Ƶ����index
    int videoindex = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return false;
    }

    AVCodecContext* pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    //4. ������Ƶ����Ϣ��codec_id�ҵ���Ӧ�Ľ�����
    AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id); //�ҵ�������
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return false;
    }
    //5.ʹ�ø�����AVCodec��ʼ��AVCodecContext
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //�򿪽�����
        printf("Could not open codec.\n");
        return false;
    }

    AVFrame* pFrame = av_frame_alloc(); //����֡��ʼ��
    unsigned char* out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1)); //�ڴ����

    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    while (av_read_frame(pFormatCtx, packet) >= 0) //��ȡý���һ֡ѹ���������ݡ����е�����av_parser_parse2()
    {
        if (packet->stream_index == videoindex)
        {
            int got_picture;
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); // ��������Ϊһ֡
            if (ret < 0) {
                printf("Decode Error.\n");
                return false;
            }
            if (got_picture) {
                int y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrame->data[0], 1, y_size, fp_out);  //Y 
                fwrite(pFrame->data[1], 1, y_size / 4, fp_out);  //U
                fwrite(pFrame->data[2], 1, y_size / 4, fp_out);  //V
                printf("Succeed to decode 1 frame!\n");
            }
        }
        av_free_packet(packet);
    }
    
    //Flush Decoder
    while (true)
    {
        int got_picture;
        int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0 || !got_picture) {
            break;
        }

        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrame->data[0], 1, y_size, fp_out);  //Y 
        fwrite(pFrame->data[1], 1, y_size / 4, fp_out);  //U
        fwrite(pFrame->data[2], 1, y_size / 4, fp_out);  //V

        printf("Flush Decoder: Succeed to decode 1 frame!\n");
    }

    fclose(fp_out);

    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return true;
}

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

/* BMP结构说明
typedef  struct  tagBITMAPFILEHEADER {
    unsigned short int  bfType;       //位图文件的类型，必须为BM
    unsigned long       bfSize;       //文件大小，以字节为单位
    unsigned short int  bfReserverd1; //位图文件保留字，必须为0
    unsigned short int  bfReserverd2; //位图文件保留字，必须为0
    unsigned long       bfbfOffBits;  //位图文件头到数据的偏移量，以字节为单位
}BITMAPFILEHEADER;

typedef  struct  tagBITMAPINFOHEADER
{
    long       biSize;               //该结构大小，字节为单位
    long       biWidth;              //图形宽度以象素为单位
    long       biHeight;             //图形高度以象素为单位
    short int  biPlanes;             //目标设备的级别，必须为1
    short int  biBitcount;           //颜色深度，每个象素所需要的位数
    short int  biCompression;        //位图的压缩类型
    long       biSizeImage;          //位图的大小，以字节为单位
    long       biXPelsPermeter;      //位图水平分辨率，每米像素数
    long       biYPelsPermeter;      //位图垂直分辨率，每米像素数
    long       biClrUsed;            //位图实际使用的颜色表中的颜色数
    long       biClrImportant;       //位图显示过程中重要的颜色数
}BITMAPINFOHEADER;

参考链接：https://www.freesion.com/article/1054806317/
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

// YUV420p：每4个 Y分量 共用一组 UV分量，前面全是Y分量，后面是U分量，再后面是V分量
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

    libyuv::ARGBToI420(rgbBuf, width * 4,// 带转换的argb数据；argb数据每一行的大小
        yDataDstPtr, yStride,            // 用于保存y分量的指针；每一行y分量的长度
        uDataDstPtr, uvStride,           // 用于保存u分量的指针；每一行u分量的长度
        vDataDstPtr, uvStride,           // 用于保存v分量的指针；每一行v分量的长度
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
    x264_param_default(pParam); //设置参数集结构体x264_param_t的缺省值
    pParam->i_width = width;                          // 视频图像的宽
    pParam->i_height = height;                        // 视频图像的高
    pParam->i_csp = csp;                              // 编码比特流的CSP
    pParam->i_frame_total = 0;                        // 编码帧的总数, 默认 0
    pParam->i_log_level = X264_LOG_DEBUG;             // 日志级别，不需要打印编码信息时直接注释掉即可
    pParam->i_threads = X264_SYNC_LOOKAHEAD_AUTO;     // 并行编码多帧; 线程数，为0则自动多线程编码
    pParam->i_keyint_max = 10;                        // 设定IDR帧之间的最间隔，在此间隔设置IDR关键帧
    pParam->i_bframe = 5;                             // 两个参考帧之间的B帧数目
    pParam->b_open_gop = 0;                           // Close GOP是指帧间的预测都是在GOP中进行的。
    pParam->i_bframe_pyramid = 0;                     // 允许部分B帧为参考帧，
    pParam->rc.i_qp_constant = 0;                     // 指定P帧的量化值，0 - 51，0表示无损
    pParam->rc.i_qp_max = 0;                          // 允许的最大量化值，默认51
    pParam->rc.i_qp_min = 0;                          // 允许的最小量化值，默认10
    pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS; // 自适应B帧判定, 可选取值：X264_B_ADAPT_FAST等
    pParam->i_fps_num = 25;                           // 帧率的分子
    pParam->i_fps_den = 1;                            // 帧率的分母
    pParam->i_timebase_num = pParam->i_fps_den;       // 时间基的分子
    pParam->i_timebase_den = pParam->i_fps_num;       // 时间基的分母
    x264_param_apply_profile(pParam, x264_profile_names[1]); // 设置编码的profile, 支持的profile在x264_profile_names中

    x264_t* pHandle = x264_encoder_open(pParam);//打开编码器
    x264_nal_t* pNals = NULL;//存储压缩编码后的码流数据
    x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));// 存储压缩编码前的像素数据
    x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));

    x264_picture_init(pPic_out);//如果调用应用程序分配自己的x264_picture_t而不是使用x264_picture_alloc，则需要执行此操作
    x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);//为图像结构体x264_picture_t分配内存

    int iNal = 0;
    int ret = x264_encoder_headers(pHandle, &pNals, &iNal);//用于输出SPS/PPS/SEI这些H.264码流的头信息

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

        ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);//编码一帧图像
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
    //flush encoder 输出编码器中剩余的码流数据
    while (1)
    {
        ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);//编码一帧图像
        if (ret == 0) {
            break;
        }
        printf("Flush 1 frame.\n");
        for (int j = 0; j < iNal; ++j) {
            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_out);
        }
        i++;
    }

    x264_picture_clean(pPic_in);//释放x264_picture_alloc()申请的资源
    x264_encoder_close(pHandle);//关闭编码器
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

    avcodec_register_all(); //注册所有的编解码器。
    //只注册编解码器有关的组件。比如说编码器、解码器、比特流滤镜等，但是不注册复用/解复用器这些和编解码器无关的组件。

    AVCodec* pCodec = avcodec_find_decoder(AV_CODEC_ID_H264); //查找解码器
    if (!pCodec) {
        printf("Codec not found\n");
        return false;
    }
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec); //为AVCodecContext分配内存
    if (!pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    AVCodecParserContext* pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264); //初始化AVCodecParserContext
    if (!pCodecParserCtx) {
        printf("Could not allocate video parser context\n");
        return false;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //打开解码器
        printf("Could not open codec\n");
        return false;
    }


    AVFrame* pFrame = av_frame_alloc(); //存储一帧解码后的像素数据
    AVPacket* packet = av_packet_alloc(); // 存储一帧（一般情况下）压缩编码数据
    //AVPacket packet; 两种初始化方式，都可以
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
            //使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据。
            int len = av_parser_parse2( // 解析获得一个Packet
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

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //解码一帧数据
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
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //解码一帧数据
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

    av_register_all();//注册了和编解码器有关的组件
    AVFormatContext* pFormatCtx = avformat_alloc_context();//创建AVFormatContext结构体

    //1. 打开媒体文件
    if (avformat_open_input(&pFormatCtx, h264FilePath.c_str(), NULL, NULL) != 0) { //打开视频流
        printf("Couldn't open input stream.\n");
        return false;
    }
    //2.读取媒体文件信息，给avFormatContext赋值
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) { //获取流信息
        printf("Couldn't find stream information.\n");
        return false;
    }
    av_dump_format(pFormatCtx, 0, h264FilePath.c_str(), 0); // 打印流信息

    //3. 匹配到视频流的index
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
    //4. 根据视频流信息的codec_id找到对应的解码器
    AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id); //找到解码器
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return false;
    }
    //5.使用给定的AVCodec初始化AVCodecContext
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //打开解码器
        printf("Could not open codec.\n");
        return false;
    }

    AVFrame* pFrame = av_frame_alloc(); //解码帧初始化
    unsigned char* out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1)); //内存分配

    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    while (av_read_frame(pFormatCtx, packet) >= 0) //获取媒体的一帧压缩编码数据。其中调用了av_parser_parse2()
    {
        if (packet->stream_index == videoindex)
        {
            int got_picture;
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); // 解码数据为一帧
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

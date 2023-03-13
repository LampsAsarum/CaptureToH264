#include "QsvH264Encoder.h"
#pragma warning(disable:4996)

QsvH264Encoder::QsvH264Encoder()
{
}

QsvH264Encoder::~QsvH264Encoder()
{
}

bool QsvH264Encoder::IsSupported()
{
	av_register_all(); //注册和编解码器有关的组件
	AVCodec* pH264Codec = avcodec_find_encoder_by_name("h264_qsv"); // 利用编码器名称查找编码器(AVCodec.name)
	if (nullptr == pH264Codec)	{
		return  false;
	}

	AVCodecContext* pEncoderContext = avcodec_alloc_context3(pH264Codec); // 为AVCodecContext结构体分配内存，并将各字段设置为默认值
	if (nullptr == pEncoderContext) {
		return false;
	}
	pEncoderContext->codec_id = pH264Codec->id;
	pEncoderContext->time_base.num = 1;
	pEncoderContext->time_base.den = 25;
	pEncoderContext->pix_fmt = *pH264Codec->pix_fmts;
	pEncoderContext->width = 1280;
	pEncoderContext->height = 720;

	bool ret = (avcodec_open2(pEncoderContext, pH264Codec, nullptr) < 0) ? false : true;
	
	avcodec_free_context(&pEncoderContext); // 释放解码器上下文
	pEncoderContext = nullptr;
	return ret;
}

bool QsvH264Encoder::OpenEncoder(int width, int height, int frameRate, int bitrate)
{
	av_register_all();
	AVCodec* pH264Codec = avcodec_find_encoder_by_name("h264_qsv"); // 利用编码器名称查找编码器(AVCodec.name)
	if (nullptr == pH264Codec) {
		return false;
	}

	m_pEncoderContext = avcodec_alloc_context3(pH264Codec); // 为AVCodecContext结构体分配内存，并将各字段设置为默认值
	if (nullptr == m_pEncoderContext) {
		return false;
	}

	m_pEncoderContext->width = width;
	m_pEncoderContext->height = height;
	m_pEncoderContext->pix_fmt = *pH264Codec->pix_fmts;
	m_pEncoderContext->framerate.num = frameRate;
	m_pEncoderContext->framerate.den = 1;
	m_pEncoderContext->gop_size = 300;
	m_pEncoderContext->has_b_frames = 0;
	m_pEncoderContext->max_b_frames = 0;
	m_pEncoderContext->codec_id = pH264Codec->id;
	m_pEncoderContext->time_base.num = 1;
	m_pEncoderContext->time_base.den = frameRate;
	m_pEncoderContext->bit_rate = bitrate; // bit_rate 单位 bps
	m_pEncoderContext->rc_max_rate = 0; //ABR

	av_opt_set(m_pEncoderContext->priv_data, "async_depth", "1", 0);
	av_opt_set(m_pEncoderContext->priv_data, "max_dec_frame_buffering", "1", 0);
	av_opt_set(m_pEncoderContext->priv_data, "look_ahead", "0", 0);
	av_opt_set(m_pEncoderContext->priv_data, "bitrate_limit", "1", 0);

	int ret = avcodec_open2(m_pEncoderContext, pH264Codec, nullptr);
	if (ret < 0)
	{
		avcodec_free_context(&m_pEncoderContext); // 释放解码器上下文
		m_pEncoderContext = nullptr;
		return false;
	}
	// 以上创建编码器成功

	m_pAVFrame = av_frame_alloc();
	if (nullptr == m_pAVFrame) {
		CloseEncoder();
		return false;
	}

	return true;
}

void QsvH264Encoder::CloseEncoder()
{
	if (m_pEncoderContext)
	{
		avcodec_free_context(&m_pEncoderContext);
		m_pEncoderContext = NULL;
	}
}

int QsvH264Encoder::Encoder(const unsigned char* inImageData, unsigned char* outH264Buffer, int outH264BufferSize)
{
	int flg = av_image_fill_arrays(m_pAVFrame->data, m_pAVFrame->linesize, inImageData,
		m_pEncoderContext->pix_fmt, m_pEncoderContext->width, m_pEncoderContext->height, 1); // 将AVFrame与图像数据关联起来
	if (flg < 0) {
		return 0;
	}

	m_pAVFrame->format = m_pEncoderContext->pix_fmt;
	m_pAVFrame->width = m_pEncoderContext->width;
	m_pAVFrame->height = m_pEncoderContext->height;

	AVPacket outAVPacket;
	av_init_packet(&outAVPacket);
	outAVPacket.data = NULL;
	outAVPacket.size = 0;

	int gotPacket = 0;
	flg = avcodec_encode_video2(m_pEncoderContext, &outAVPacket, m_pAVFrame, &gotPacket);
	if (flg == 0 && gotPacket != 0 && outAVPacket.size < outH264BufferSize)
	{
		memcpy(outH264Buffer, outAVPacket.data, outAVPacket.size);
		int size = outAVPacket.size;
		av_packet_unref(&outAVPacket);
		return size;
	}
	else
	{
		av_packet_unref(&outAVPacket);
		return 0;
	}
}


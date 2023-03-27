#include "QsvH264Encoder.h"
#pragma warning(disable:4996)

namespace
{
	const char* AVCFindStartCodeInternal(const char* p, const char* end)
	{
		const char* a = p + 4 - ((intptr_t)p & 3);

		for (end -= 3; p < a && p < end; p++) {
			if (p[0] == 0 && p[1] == 0 && p[2] == 1)
				return p;
		}

		for (end -= 3; p < end; p += 4) {
			unsigned int x = *(const unsigned int*)p;
			//      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
			//      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
			if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
				if (p[1] == 0) {
					if (p[0] == 0 && p[2] == 1)
						return p;
					if (p[2] == 0 && p[3] == 1)
						return p + 1;
				}
				if (p[3] == 0) {
					if (p[2] == 0 && p[4] == 1)
						return p + 2;
					if (p[4] == 0 && p[5] == 1)
						return p + 3;
				}
			}
		}

		for (end += 3; p < end; p++) {
			if (p[0] == 0 && p[1] == 0 && p[2] == 1)
				return p;
		}

		return end + 3;

	}

	static const char* AVCFindStartCode(const char* p, const char* end)
	{
		const char* out = AVCFindStartCodeInternal(p, end);
		if (p < out && out < end && !out[-1]) out--;
		return out;
	}

	enum {
		NAL_SLICE = 1,
		NAL_DPA = 2,
		NAL_DPB = 3,
		NAL_DPC = 4,
		NAL_IDR_SLICE = 5,
		NAL_SEI = 6,
		NAL_SPS = 7,
		NAL_PPS = 8,
		NAL_AUD = 9,
		NAL_END_SEQUENCE = 10,
		NAL_END_STREAM = 11,
		NAL_FILLER_DATA = 12,
		NAL_SPS_EXT = 13,
		NAL_AUXILIARY_SLICE = 19,
		NAL_FF_IGNORE = 0xff0f001,
	};

	//将一帧码流中不需要的NALU过滤，只保留基本的NALU，用于网络传输时节省带宽
	//过滤的NALU包括：NAL_SEI、NAL_AUD、NAL_FILLER_DATA
	bool AVCFilterNalUnits(const char* bufIn, int inSize, char* bufOut, int* outSize)
	{
		const char* p = bufIn;
		const char* end = p + inSize;
		const char* nal_start, * nal_end;

		char* pbuf = bufOut;
		bool bFoundSps = false;
		unsigned int start_code_len = 0;

		static unsigned char byHeadLong[] = { 0x00, 0x00, 0x00, 0x01 };
		static unsigned char byHeadShort[] = { 0x00, 0x00, 0x01 };

		*outSize = 0;
		nal_start = AVCFindStartCode(p, end);
		while (nal_start < end)
		{
			start_code_len = 0;
			while (!*(nal_start++))
			{
				start_code_len++;
			}

			nal_end = AVCFindStartCode(nal_start, end);

			unsigned int nal_size = (unsigned int)(nal_end - nal_start);
			unsigned int nal_type = nal_start[0] & 0x1f;

			if (nal_type == NAL_SPS)
			{
				bFoundSps = true;
			}

			//SEI AUD 以及部分编码器为了实现CBR而填充的NALU均可以过滤
			//部分编码器每帧都附带了PPS，可以过滤，仅保留IDR帧SPS一起的PPS
			if ((nal_type != NAL_SEI) && (nal_type != NAL_AUD) && (nal_type != NAL_FILLER_DATA))
			{
				if ((nal_type != NAL_PPS) || (bFoundSps == true))
				{
					//先复原起始码
					if (start_code_len == 3)
					{
						//长头
						memcpy(pbuf, byHeadLong, sizeof(byHeadLong));
						pbuf += sizeof(byHeadLong);
					}
					else
					{
						//短头
						memcpy(pbuf, byHeadShort, sizeof(byHeadShort));
						pbuf += sizeof(byHeadShort);
					}

					memcpy(pbuf, nal_start, nal_size);
					pbuf += nal_size;
				}

			}

			nal_start = nal_end;
		}

		*outSize = (int)(pbuf - bufOut);
		if (*outSize == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

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

	m_InputPixelFormat = *pH264Codec->pix_fmts;

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
		int size = 0;
		bool bRet = AVCFilterNalUnits((const char*)outAVPacket.data, outAVPacket.size, (char*)outH264Buffer, &size);
		if (bRet == false) {
			size = 0;
		}
		av_packet_unref(&outAVPacket);
		return size;
	}
	else
	{
		av_packet_unref(&outAVPacket);
		return 0;
	}
}


#include "X264Encoder.h"
#include <memory>

X264Encoder::X264Encoder(int width, int height)
{
    memset(&m_pic_in, 0, sizeof(m_pic_in));

    x264_param_t x264Param;
    x264_param_default(&x264Param);
    x264_param_default_preset(&x264Param, "ultrafast", "zerolatency");

    x264Param.i_threads = X264_SYNC_LOOKAHEAD_AUTO;     // ���б����֡; �߳�����Ϊ0���Զ����̱߳���
    x264Param.i_width = width;                          // ��Ƶͼ��Ŀ�
    x264Param.i_height = height;                        // ��Ƶͼ��ĸ�
    x264Param.i_csp = X264_CSP_I420;                    // �����������CSP
    x264Param.i_fps_num = 25;                           // ֡�ʵķ���
    x264Param.i_fps_den = 1;                            // ֡�ʵķ�ĸ
    x264Param.i_timebase_num = x264Param.i_fps_den;     // ʱ����ķ���
    x264Param.i_timebase_den = x264Param.i_fps_num;     // ʱ����ķ�ĸ
    x264Param.i_bframe = 0;                             // �����ο�֮֡���B֡��Ŀ
    x264Param.i_keyint_min = X264_KEYINT_MIN_AUTO;
    x264Param.i_keyint_max = X264_KEYINT_MAX_INFINITE;  // �趨IDR֮֡�����������ڴ˼������IDR�ؼ�֡
    x264Param.rc.i_rc_method = X264_RC_ABR;             // ���ʿ��ƣ�CQP(�㶨����)��CRF(�㶨����)��ABR(ƽ������)
    x264Param.rc.i_bitrate = 2 * width * height * 3 / 8 / 1024;
    x264Param.i_log_level = X264_LOG_NONE;

    x264_param_apply_profile(&x264Param, x264_profile_names[1]);

    m_pHandle = x264_encoder_open(&x264Param);

    x264_picture_init(&m_pic_in);
    /*m_pic_in.img.i_csp = X264_CSP_I420;
    m_pic_in.img.i_stride[0] = width;
    m_pic_in.img.i_stride[1] = width / 2;
    m_pic_in.img.i_stride[2] = width / 2;*/

    x264_picture_alloc(&m_pic_in, X264_CSP_I420, width, height);//Ϊͼ��ṹ��x264_picture_t�����ڴ�
}

X264Encoder::~X264Encoder()
{
}

bool X264Encoder::EncodeFrame(uint8_t* in_buf[3], uint8_t* out_ppData[8], size_t out_linesize[8], bool in_forceKey)
{
    if (in_buf != NULL) {//encode
        m_pic_in.i_type = in_forceKey ? X264_TYPE_IDR : X264_TYPE_AUTO;
        m_pic_in.img.plane[0] = in_buf[0];
        m_pic_in.img.plane[1] = in_buf[1];
        m_pic_in.img.plane[2] = in_buf[2];
        return Encode(&m_pic_in, out_ppData, out_linesize);
    }
    return Encode(NULL, out_ppData, out_linesize);
}

bool X264Encoder::Encode(x264_picture_t* pic_in, uint8_t* out_ppData[8], size_t out_linesize[8])
{
    if (!m_pHandle) {
        return false;
    }

    if (out_linesize == nullptr || out_ppData == nullptr) {
        return false;
    }

    if (pic_in != NULL) {//encode
        int num = 0;
        x264_nal_t* nals;
        x264_picture_t pic_out;
        if (x264_encoder_encode(m_pHandle, &nals, &num, &m_pic_in, &pic_out) < 0) {
            return false;
        }

        int index = 0;
        for (x264_nal_t* nal = nals; nal < nals + num; ++nal, ++index) {
            out_linesize[index] = nal->i_payload;
            if (out_linesize[index] > 0) {
                out_ppData[index] = new uint8_t[out_linesize[index]];
                memcpy_s(out_ppData[index], out_linesize[index], nal->p_payload, nal->i_payload);
            }
        }
        out_ppData[index] = nullptr;
    }
    else {//flush
        int i_nal;
        int index = 0;
        x264_nal_t* nal;
        x264_picture_t pic_out;
        for (; x264_encoder_delayed_frames(m_pHandle) >= 1 && index < 8; ++index) {
            // nals bitstream is sequential in memory
            int frame_size = x264_encoder_encode(m_pHandle, &nal, &i_nal, NULL, &pic_out);
            if (frame_size == 0) {
                if (index == 0) {
                    return false;
                }
                else {
                    out_ppData[index] = NULL;
                    return true;
                }
            }
            else {
                out_linesize[index] = frame_size;
                if (out_linesize[index] > 0) {
                    out_ppData[index] = new uint8_t[out_linesize[index]];
                    memcpy_s(out_ppData[index], out_linesize[index], nal->p_payload, frame_size);
                }
            }
        }
    }

    return true;
}

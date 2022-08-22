#pragma once
#include <string>

class Convert
{
public:
    /// <summary>
    /// Rgb24��ʽͼƬתΪBmp��ʽ
    /// </summary>
    /// <param name="rgbBuf">rgb����</param>
    /// <param name="width">rgbͼ����</param>
    /// <param name="height">rgbͼ��߶�</param>
    /// <param name="bmpBuf">���bmpͼ������</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool Rgb24ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize);

    /// <summary>
    /// Rgb32��ʽͼƬתΪBmp��ʽ
    /// </summary>
    /// <param name="rgbBuf">rgb����</param>
    /// <param name="width">rgbͼ����</param>
    /// <param name="height">rgbͼ��߶�</param>
    /// <param name="bmpBuf">���bmpͼ������</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool Rgb32ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize);

    /// <summary>
    /// Rgb24תΪYuv420
    /// </summary>
    /// <param name="rgbBuf">rgb����</param>
    /// <param name="width">rgbͼ����</param>
    /// <param name="height">rgbͼ��߶�</param>
    /// <param name="yuvBuf">���yuvͼ������</param>
    /// <param name="yuvBufferSize">����yuvBuf��С</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool Rgb24ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize);

    /// <summary>
    /// Rgb32תΪYuv420
    /// </summary>
    /// <param name="rgbBuf">rgb����</param>
    /// <param name="width">rgbͼ����</param>
    /// <param name="height">rgbͼ��߶�</param>
    /// <param name="yuvBuf">���yuvͼ������</param>
    /// <param name="yuvBufferSize">����yuvBuf��С</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool Rgb32ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize);

    /// <summary>
    /// Yuv420�ļ�תH264�ļ�
    /// </summary>
    /// <param name="yuvFilePath">YUV�ļ�·��</param>
    /// <param name="h264FilePath">H264�ļ�·��</param>
    /// <param name="width">��Ƶ���</param>
    /// <param name="height">��Ƶ�߶�</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height);
    
    /// <summary>
    /// H264�ļ�תYuv420�ļ�
    /// </summary>
    /// <param name="h264FilePath">H264�ļ�·��</param>
    /// <param name="yuvFilePath">YUV�ļ�·��</param>
    /// <param name="width">��Ƶ���</param>
    /// <param name="height">��Ƶ�߶�</param>
    /// <returns>�ɹ�����true��ʧ�ܷ���false</returns>
    static bool H264ToYUV420(std::string h264FilePath, std::string yuvFilePath, int width, int height);
};


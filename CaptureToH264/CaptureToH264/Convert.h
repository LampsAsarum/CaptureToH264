#pragma once
#include <string>

class Convert
{
public:
    /// <summary>
    /// Rgb24格式图片转为Bmp格式
    /// </summary>
    /// <param name="rgbBuf">rgb数据</param>
    /// <param name="width">rgb图像宽度</param>
    /// <param name="height">rgb图像高度</param>
    /// <param name="bmpBuf">输出bmp图像数据</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool Rgb24ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize);

    /// <summary>
    /// Rgb32格式图片转为Bmp格式
    /// </summary>
    /// <param name="rgbBuf">rgb数据</param>
    /// <param name="width">rgb图像宽度</param>
    /// <param name="height">rgb图像高度</param>
    /// <param name="bmpBuf">输出bmp图像数据</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool Rgb32ToBmp(unsigned char* rgbBuf, const int width, const int height, unsigned char* bmpBuf, const int bmpBufferSize);

    /// <summary>
    /// Rgb24转为Yuv420
    /// </summary>
    /// <param name="rgbBuf">rgb数据</param>
    /// <param name="width">rgb图像宽度</param>
    /// <param name="height">rgb图像高度</param>
    /// <param name="yuvBuf">输出yuv图像数据</param>
    /// <param name="yuvBufferSize">输入yuvBuf大小</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool Rgb24ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize);

    /// <summary>
    /// Rgb32转为Yuv420
    /// </summary>
    /// <param name="rgbBuf">rgb数据</param>
    /// <param name="width">rgb图像宽度</param>
    /// <param name="height">rgb图像高度</param>
    /// <param name="yuvBuf">输出yuv图像数据</param>
    /// <param name="yuvBufferSize">输入yuvBuf大小</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool Rgb32ToYUV420(unsigned char* rgbBuf, const int width, const int height, unsigned char* yuvBuf, const int yuvBufferSize);

    /// <summary>
    /// Yuv420文件转H264文件
    /// </summary>
    /// <param name="yuvFilePath">YUV文件路径</param>
    /// <param name="h264FilePath">H264文件路径</param>
    /// <param name="width">视频宽度</param>
    /// <param name="height">视频高度</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool YUV420ToH264(std::string yuvFilePath, std::string h264FilePath, int width, int height);
    
    /// <summary>
    /// H264文件转Yuv420文件
    /// </summary>
    /// <param name="h264FilePath">H264文件路径</param>
    /// <param name="yuvFilePath">YUV文件路径</param>
    /// <param name="width">视频宽度</param>
    /// <param name="height">视频高度</param>
    /// <returns>成功返回true，失败返回false</returns>
    static bool H264ToYUV420(std::string h264FilePath, std::string yuvFilePath, int width, int height);
};


#pragma once

// Windows ͼ���豸�ӿ�(GDI)��Ϊ���豸�޹ص�ͼ����Ƶġ����� Windows ��Ӧ�ó�����ֱ�ӷ���ͼ��Ӳ����
// Ӧ�ó���ͨ�� GDI �����豸����������н�����GDI ��ͼ����ͨ����Ļ���豸����(DC)��ȡ����ǰ��Ļ��λͼ���ݡ�

class GDICapture
{
public:
    static bool CaptureRgb24(unsigned char* rgbBuffer, int bufferSize);
    static bool CaptureRgb32(unsigned char* rgbBuffer, int bufferSize);
};


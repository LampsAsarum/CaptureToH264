#pragma once

// ͨ��GPU���������ܽ�GDI�ߺܶ࣬����ֻ��32λRGB���ݡ�

class IDXGIOutputDuplication;
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Texture2D;

class DXGICapture
{
public:
    DXGICapture();
    bool CaptureRgb32(unsigned char* rgbBuffer, const int rgbBufferSize);

private:
    IDXGIOutputDuplication* m_pDXGIOutputDuplication;
    ID3D11Device* m_pDX11Device;
    ID3D11DeviceContext* m_pDX11DeviceContext;

    // ��ͼ
    ID3D11Texture2D* m_pCopyBuffer;
    ID3D11Texture2D* m_pDX11Texture;
};


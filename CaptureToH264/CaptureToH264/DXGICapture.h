#pragma once

// ͨ��GPU���������ܽ�GDI�ߺܶ࣬����ֻ��32λRGB���ݡ�

class IDXGIOutputDuplication;
class ID3D11Device;
class ID3D11DeviceContext;

class DXGICapture
{
public:
    DXGICapture();
    bool CaptureRgb32(unsigned char** rgbBuffer, const int rgbBufferSize);

private:
    IDXGIOutputDuplication* m_pDXGIOutputDup;
    ID3D11Device* m_pDX11Dev;
    ID3D11DeviceContext* m_pDX11DevCtx;
};

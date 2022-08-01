#pragma once

// 通过GPU截屏，性能较GDI高很多，并且只有32位RGB数据。

class IDXGIOutputDuplication;
class ID3D11Device;
class ID3D11DeviceContext;

class DXGICapture
{
public:
    DXGICapture();
    int CaptureRgb32(unsigned char** rgbBuffer);

private:
    IDXGIOutputDuplication* m_pDXGIOutputDup;
    ID3D11Device* m_pDX11Dev;
    ID3D11DeviceContext* m_pDX11DevCtx;
};


#pragma once

#include <d3d9.h>
#pragma comment(lib,"D3D9.lib")

class Direct3DRenderer
{
public:
    Direct3DRenderer();
    ~Direct3DRenderer();

    bool Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight);
    void Cleanup();
    bool Render(unsigned char* buffer);

private:
    int                     m_width;
    int                     m_height;
    RECT                    m_rtViewport;

    CRITICAL_SECTION        m_critial;
    IDirect3D9*             m_pDirect3D9;
    IDirect3DDevice9*       m_pDirect3DDevice;
    IDirect3DSurface9*      m_pDirect3DSurfaceRender;
};


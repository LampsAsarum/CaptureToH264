#include "Direct3DRender.h"

Direct3DRender::Direct3DRender()
{
    m_width = 0;
    m_height = 0;
    m_rtViewport = {};
    m_critial = {};
    m_pDirect3D9 = nullptr;
    m_pDirect3DDevice = nullptr;
    m_pDirect3DSurfaceRender = nullptr;
}

Direct3DRender::~Direct3DRender()
{
}

bool Direct3DRender::Init(HWND hwnd, unsigned int width, unsigned int height)
{
	HRESULT lRet;
	InitializeCriticalSection(&m_critial);
	Cleanup();

	m_pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pDirect3D9 == NULL) {
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	GetClientRect(hwnd, &m_rtViewport);

	lRet = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pDirect3DDevice);
	if (FAILED(lRet)) {
		return false;
	}

	m_width = width;
	m_height = height;
	lRet = m_pDirect3DDevice->CreateOffscreenPlainSurface(width, height, (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_DEFAULT, &m_pDirect3DSurfaceRender, NULL);

	if (FAILED(lRet)) {
		return false;
	}

	return 0;
}

void Direct3DRender::Cleanup()
{
    EnterCriticalSection(&m_critial);
    if (m_pDirect3DSurfaceRender) {
        m_pDirect3DSurfaceRender->Release();
    }
    if (m_pDirect3DDevice) {
        m_pDirect3DDevice->Release();
    }
    if (m_pDirect3D9) {
        m_pDirect3D9->Release();
    }
    LeaveCriticalSection(&m_critial);
}

bool Direct3DRender::Render(unsigned char* buffer)
{
	if (m_pDirect3DSurfaceRender == NULL) {
		return false;
	}

	HRESULT lRet;
	D3DLOCKED_RECT d3d_rect;
	lRet = m_pDirect3DSurfaceRender->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
	if (FAILED(lRet)) {
		return false;
	}

	BYTE* pSrc = buffer;
	BYTE* pDest = (BYTE*)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;
	unsigned long i = 0;

	for (i = 0; i < m_height; i++) {
		memcpy(pDest + i * stride, pSrc + i * m_width, m_width);
	}
	for (i = 0; i < m_height / 2; i++) {
		memcpy(pDest + stride * m_height + i * stride / 2, pSrc + m_width * m_height + m_width * m_height / 4 + i * m_width / 2, m_width / 2);
	}
	for (i = 0; i < m_height / 2; i++) {
		memcpy(pDest + stride * m_height + stride * m_height / 4 + i * stride / 2, pSrc + m_width * m_height + i * m_width / 2, m_width / 2);
	}

	lRet = m_pDirect3DSurfaceRender->UnlockRect();
	if (FAILED(lRet)) {
		return -1;
	}

	if (m_pDirect3DDevice == NULL) {
		return -1;
	}

	m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_pDirect3DDevice->BeginScene();
	IDirect3DSurface9* pBackBuffer = NULL;

	m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender, NULL, pBackBuffer, &m_rtViewport, D3DTEXF_LINEAR);
	m_pDirect3DDevice->EndScene();
	m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
	pBackBuffer->Release();

	return true;
}
#include "DXGICapture.h"
#include <stdio.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

DXGICapture::DXGICapture()
    : m_pDXGIOutputDup(nullptr)
    , m_pDX11Dev(nullptr)
    , m_pDX11DevCtx(nullptr)
{
    HRESULT hr = S_OK;

    //Driver types supported ֧�ֵ��豸����
    //����˵�� https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_driver_type
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,	//Ӳ��������Ӳ��֧������Direct3D����
        D3D_DRIVER_TYPE_WARP,		//���������������
        D3D_DRIVER_TYPE_REFERENCE	//������������ȸߣ��ٶ���
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported ֧�ֵĹ��ܼ���
    //����Direct3D�豸����ԵĹ��ܼ� https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_feature_level
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,		//Direct3D 11.0֧�ֵ�Ŀ�깦�ܣ�������ɫ��ģ��5
        D3D_FEATURE_LEVEL_10_1,		//Direct3D 10.1֧�ֵ�Ŀ�깦�ܣ�������ɫ��ģ��4
        D3D_FEATURE_LEVEL_10_0,		//Direct3D 10.0֧�ֵ�Ŀ�깦�ܣ�������ɫ��ģ��4
        D3D_FEATURE_LEVEL_9_1		//Ŀ�깦��[���ܼ���]��/windows/desktop/direct3d11/overviews-direct3d-11-devices-downlevel-intro��9.1֧�֣�������ɫ��ģ��2
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

    D3D_FEATURE_LEVEL FeatureLevel;
    m_pDX11Dev = nullptr;
    m_pDX11DevCtx = nullptr;

    //1.Create D3D device ����D3D�豸
    for (UINT driverTypeIndex = 0; driverTypeIndex < NumDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[driverTypeIndex], nullptr, 0,
            FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION,
            &m_pDX11Dev, &FeatureLevel, &m_pDX11DevCtx);
        if (SUCCEEDED(hr)) {
            break;
        }
    }

    //2.Get DXGI device ��ȡ DXGI �豸
    IDXGIDevice* pDXGIDev = nullptr;
    hr = m_pDX11Dev->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDev));
    if (FAILED(hr)) {
        return;
    }

    //3.Get DXGI adapter ��ȡ DXGI ������
    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDev->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));
    if (FAILED(hr)) {
        return;
    }

    //4.Get output ��ȡ���, ö��������(��Ƶ��)���
    UINT i = 0;
    IDXGIOutput* pDXGIOutput = nullptr;
    hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);
    if (FAILED(hr)) {
        return;
    }

    //5.Get output description struct ��ȡ��������ṹ
    DXGI_OUTPUT_DESC DesktopDesc;
    hr = pDXGIOutput->GetDesc(&DesktopDesc);
    if (FAILED(hr)) {
        return;
    }

    //6.QI for Output1 ����ӿ�Output1
    IDXGIOutput1* pDXGIOutput1 = nullptr;
    hr = pDXGIOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pDXGIOutput1));
    if (FAILED(hr)) {
        return;
    }

    //7.Create desktop duplication �������渱��
    m_pDXGIOutputDup = nullptr;
    hr = pDXGIOutput1->DuplicateOutput(m_pDX11Dev, &m_pDXGIOutputDup);
    if (FAILED(hr)) {
        return;
    }
}

bool DXGICapture::CaptureRgb32(unsigned char** rgbBuffer, const int rgbBufferSize)
{
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_pDXGIOutputDup->AcquireNextFrame(20, &frameInfo, &desktopResource);
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            if (desktopResource) {
                desktopResource->Release();
                desktopResource = nullptr;
            }
            hr = m_pDXGIOutputDup->ReleaseFrame();
        }
        else {
            return false;
        }
    }

    //Query next frame staging buffer ��ѯ��һ֡�ݴ滺����
    ID3D11Texture2D* pDX11Texture = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pDX11Texture));
    desktopResource->Release();
    desktopResource = nullptr;
    if (FAILED(hr)) {
        return false;
    }

    ID3D11Texture2D* pCopyBuffer = nullptr;

    //Copy old description ���ƾ�����
    D3D11_TEXTURE2D_DESC desc;
    if (pDX11Texture) {
        pDX11Texture->GetDesc(&desc);
    }
    else if (pCopyBuffer) {
        pCopyBuffer->GetDesc(&desc);
    }
    else {
        return false;
    }

    //Create a new staging buffer for fill frame image Ϊ���֡ͼ�񴴽�һ���µ��ݴ滺����
    if (pCopyBuffer == nullptr) {
        D3D11_TEXTURE2D_DESC CopyBufferDesc;
        CopyBufferDesc.Width = desc.Width;
        CopyBufferDesc.Height = desc.Height;
        CopyBufferDesc.MipLevels = 1;
        CopyBufferDesc.ArraySize = 1;
        CopyBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        CopyBufferDesc.SampleDesc.Count = 1;
        CopyBufferDesc.SampleDesc.Quality = 0;
        CopyBufferDesc.Usage = D3D11_USAGE_STAGING;
        CopyBufferDesc.BindFlags = 0;
        CopyBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        CopyBufferDesc.MiscFlags = 0;

        hr = m_pDX11Dev->CreateTexture2D(&CopyBufferDesc, nullptr, &pCopyBuffer);
        if (FAILED(hr)) {
            return false;
        }
    }

    //Copy next staging buffer to new staging buffer ����һ���ݴ滺�������Ƶ��µ��ݴ滺����
    if (pDX11Texture) {
        m_pDX11DevCtx->CopyResource(pCopyBuffer, pDX11Texture);
    }

    //Create staging buffer for map bits Ϊӳ��λ�����ݴ滺����
    IDXGISurface* CopySurface = nullptr;
    hr = pCopyBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&CopySurface);
    if (FAILED(hr)) {
        return false;
    }

    //Copy bits to user space ��λ���Ƶ��û��ռ�
    DXGI_MAPPED_RECT MappedSurface;
    hr = CopySurface->Map(&MappedSurface, DXGI_MAP_READ);

    if (SUCCEEDED(hr)) {
        for (int i = 0; i < 1080; ++i) {
            memcpy(*rgbBuffer + i * MappedSurface.Pitch, MappedSurface.pBits + i * MappedSurface.Pitch, MappedSurface.Pitch);
        }
        CopySurface->Unmap();
    }

    CopySurface->Unmap();
    hr = CopySurface->Release();
    CopySurface = nullptr;

    if (m_pDXGIOutputDup) {
        hr = m_pDXGIOutputDup->ReleaseFrame();
    }
    return true;
}

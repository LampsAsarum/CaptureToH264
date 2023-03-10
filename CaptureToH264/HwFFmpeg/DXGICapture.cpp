#include "DXGICapture.h"
#include <stdio.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define RELEASE_OBJECT(obj) {if(obj) obj->Release(); obj = nullptr;}

DXGICapture::DXGICapture()
    : m_pDXGIOutputDuplication(nullptr)
    , m_pDX11Device(nullptr)
    , m_pDX11DeviceContext(nullptr)
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

    //Feature levels supported ֧�ֵĹ��ܼ���
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
    m_pDX11Device = nullptr;
    m_pDX11DeviceContext = nullptr;

    //1.Create D3D device ����D3D�豸
    for (UINT driverTypeIndex = 0; driverTypeIndex < NumDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[driverTypeIndex], nullptr, 0,
            FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION,
            &m_pDX11Device, &FeatureLevel, &m_pDX11DeviceContext);
        if (SUCCEEDED(hr)) {
            break;
        }
    }

    //2.Get DXGI device ��ȡ DXGI �豸
    IDXGIDevice* pDXGIDevice = nullptr;
    hr = m_pDX11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));
    if (FAILED(hr)) {
        return;
    }

    //3.Get DXGI adapter ��ȡ DXGI ������
    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));
    RELEASE_OBJECT(pDXGIDevice);
    if (FAILED(hr)) {
        return;
    }

    //4.Get output ��ȡ���, ö��������(��Ƶ��)���
    UINT i = 0;
    IDXGIOutput* pDXGIOutput = nullptr;
    hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);
    RELEASE_OBJECT(pDXGIAdapter);
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
    RELEASE_OBJECT(pDXGIOutput);
    if (FAILED(hr)) {
        return;
    }

    //7.Create desktop duplication �������渱��
    m_pDXGIOutputDuplication = nullptr;
    hr = pDXGIOutput1->DuplicateOutput(m_pDX11Device, &m_pDXGIOutputDuplication);
    RELEASE_OBJECT(pDXGIOutput1);
    if (FAILED(hr)) {
        return;
    }
}

bool DXGICapture::CaptureRgb32(unsigned char* rgbBuffer, const int rgbBufferSize)
{
    IDXGIResource* pDesktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_pDXGIOutputDuplication->AcquireNextFrame(20, &frameInfo, &pDesktopResource);
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            if (pDesktopResource) {
                RELEASE_OBJECT(pDesktopResource);
            }
            hr = m_pDXGIOutputDuplication->ReleaseFrame();
        }
        else {
            return false;
        }
    }

    //Query next frame staging buffer ��ѯ��һ֡�ݴ滺����
    if (m_pDX11Texture)
    {
        RELEASE_OBJECT(m_pDX11Texture);
    }

    if (pDesktopResource) {
        hr = pDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pDX11Texture));
        RELEASE_OBJECT(pDesktopResource);
        if (FAILED(hr)) {
            return false;
        }
    }

    //Copy old description ���ƾ�����
    D3D11_TEXTURE2D_DESC frameDescriptor;
    if (m_pDX11Texture) {
        m_pDX11Texture->GetDesc(&frameDescriptor);
    }
    else if (m_pCopyBuffer) {
        m_pCopyBuffer->GetDesc(&frameDescriptor);
    }
    else {
        return false;
    }

    //Create a new staging buffer for fill frame image Ϊ���֡ͼ�񴴽�һ���µ��ݴ滺����
    if (m_pCopyBuffer == nullptr) {
        D3D11_TEXTURE2D_DESC copyBufferDesc;
        copyBufferDesc.Width = frameDescriptor.Width;
        copyBufferDesc.Height = frameDescriptor.Height;
        copyBufferDesc.MipLevels = 1;
        copyBufferDesc.ArraySize = 1;
        copyBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        copyBufferDesc.SampleDesc.Count = 1;
        copyBufferDesc.SampleDesc.Quality = 0;
        copyBufferDesc.Usage = D3D11_USAGE_STAGING;
        copyBufferDesc.BindFlags = 0;
        copyBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        copyBufferDesc.MiscFlags = 0;

        hr = m_pDX11Device->CreateTexture2D(&copyBufferDesc, nullptr, &m_pCopyBuffer);
        if (FAILED(hr)) {
            RELEASE_OBJECT(m_pDX11Texture);
            m_pDXGIOutputDuplication->ReleaseFrame();
            return false;
        }
    }

    //Copy next staging buffer to new staging buffer ����һ���ݴ滺�������Ƶ��µ��ݴ滺����
    if (m_pDX11Texture) {
        m_pDX11DeviceContext->CopyResource(m_pCopyBuffer, m_pDX11Texture);
        RELEASE_OBJECT(m_pDX11Texture);
        m_pDXGIOutputDuplication->ReleaseFrame();
    }

    //Create staging buffer for map bits Ϊӳ��λ�����ݴ滺����
    IDXGISurface* CopySurface = nullptr;
    hr = m_pCopyBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&CopySurface);
    if (FAILED(hr)) {
        return false;
    }

    //Copy bits to user space ��λ���Ƶ��û��ռ�
    DXGI_MAPPED_RECT MappedSurface;
    hr = CopySurface->Map(&MappedSurface, DXGI_MAP_READ);

    if (SUCCEEDED(hr)) {
        memcpy(rgbBuffer, MappedSurface.pBits, rgbBufferSize);
        CopySurface->Unmap();
    }
    RELEASE_OBJECT(CopySurface);

    if (m_pDXGIOutputDuplication) {
        m_pDXGIOutputDuplication->ReleaseFrame();
    }

    return true;
}

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

    //Driver types supported 支持的设备类型
    //类型说明 https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_driver_type
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,	//硬件驱动，硬件支持所有Direct3D功能
        D3D_DRIVER_TYPE_WARP,		//软件驱动，高性能
        D3D_DRIVER_TYPE_REFERENCE	//软件驱动，精度高，速度慢
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    //Feature levels supported 支持的功能级别
    //描述Direct3D设备所针对的功能集 https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_feature_level
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,		//Direct3D 11.0支持的目标功能，包括着色器模型5
        D3D_FEATURE_LEVEL_10_1,		//Direct3D 10.1支持的目标功能，包括着色器模型4
        D3D_FEATURE_LEVEL_10_0,		//Direct3D 10.0支持的目标功能，包括着色器模型4
        D3D_FEATURE_LEVEL_9_1		//目标功能[功能级别]（/windows/desktop/direct3d11/overviews-direct3d-11-devices-downlevel-intro）9.1支持，包括着色器模型2
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

    D3D_FEATURE_LEVEL FeatureLevel;
    m_pDX11Device = nullptr;
    m_pDX11DeviceContext = nullptr;

    //1.Create D3D device 创建D3D设备
    for (UINT driverTypeIndex = 0; driverTypeIndex < NumDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[driverTypeIndex], nullptr, 0,
            FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION,
            &m_pDX11Device, &FeatureLevel, &m_pDX11DeviceContext);
        if (SUCCEEDED(hr)) {
            break;
        }
    }

    //2.Get DXGI device 获取 DXGI 设备
    IDXGIDevice* pDXGIDevice = nullptr;
    hr = m_pDX11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));
    if (FAILED(hr)) {
        return;
    }

    //3.Get DXGI adapter 获取 DXGI 适配器
    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));
    RELEASE_OBJECT(pDXGIDevice);
    if (FAILED(hr)) {
        return;
    }

    //4.Get output 获取输出, 枚举适配器(视频卡)输出
    UINT i = 0;
    IDXGIOutput* pDXGIOutput = nullptr;
    hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);
    RELEASE_OBJECT(pDXGIAdapter);
    if (FAILED(hr)) {
        return;
    }

    //5.Get output description struct 获取输出描述结构
    DXGI_OUTPUT_DESC DesktopDesc;
    hr = pDXGIOutput->GetDesc(&DesktopDesc);
    if (FAILED(hr)) {
        return;
    }

    //6.QI for Output1 请求接口Output1
    IDXGIOutput1* pDXGIOutput1 = nullptr;
    hr = pDXGIOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pDXGIOutput1));
    RELEASE_OBJECT(pDXGIOutput);
    if (FAILED(hr)) {
        return;
    }

    //7.Create desktop duplication 创建桌面副本
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

    //Query next frame staging buffer 查询下一帧暂存缓冲区
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

    //Copy old description 复制旧描述
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

    //Create a new staging buffer for fill frame image 为填充帧图像创建一个新的暂存缓冲区
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

    //Copy next staging buffer to new staging buffer 将下一个暂存缓冲区复制到新的暂存缓冲区
    if (m_pDX11Texture) {
        m_pDX11DeviceContext->CopyResource(m_pCopyBuffer, m_pDX11Texture);
        RELEASE_OBJECT(m_pDX11Texture);
        m_pDXGIOutputDuplication->ReleaseFrame();
    }

    //Create staging buffer for map bits 为映射位创建暂存缓冲区
    IDXGISurface* CopySurface = nullptr;
    hr = m_pCopyBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&CopySurface);
    if (FAILED(hr)) {
        return false;
    }

    //Copy bits to user space 将位复制到用户空间
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

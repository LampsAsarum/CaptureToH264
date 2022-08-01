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

    //Driver types supported 支持的设备类型
    //类型说明 https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_driver_type
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,	//硬件驱动，硬件支持所有Direct3D功能
        D3D_DRIVER_TYPE_WARP,		//软件驱动，高性能
        D3D_DRIVER_TYPE_REFERENCE	//软件驱动，精度高，速度慢
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported 支持的功能级别
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
    m_pDX11Dev = nullptr;
    m_pDX11DevCtx = nullptr;

    //1.Create D3D device 创建D3D设备
    for (UINT driverTypeIndex = 0; driverTypeIndex < NumDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[driverTypeIndex], nullptr, 0,
            FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION,
            &m_pDX11Dev, &FeatureLevel, &m_pDX11DevCtx);
        if (SUCCEEDED(hr)) {
            break;
        }
    }

    //2.Get DXGI device 获取 DXGI 设备
    IDXGIDevice* pDXGIDev = nullptr;
    hr = m_pDX11Dev->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDev));
    if (FAILED(hr)) {
        return;
    }

    //3.Get DXGI adapter 获取 DXGI 适配器
    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDev->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));
    if (FAILED(hr)) {
        return;
    }

    //4.Get output 获取输出, 枚举适配器(视频卡)输出
    UINT i = 0;
    IDXGIOutput* pDXGIOutput = nullptr;
    hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);
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
    if (FAILED(hr)) {
        return;
    }

    //7.Create desktop duplication 创建桌面副本
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

    //Query next frame staging buffer 查询下一帧暂存缓冲区
    ID3D11Texture2D* pDX11Texture = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pDX11Texture));
    desktopResource->Release();
    desktopResource = nullptr;
    if (FAILED(hr)) {
        return false;
    }

    ID3D11Texture2D* pCopyBuffer = nullptr;

    //Copy old description 复制旧描述
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

    //Create a new staging buffer for fill frame image 为填充帧图像创建一个新的暂存缓冲区
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

    //Copy next staging buffer to new staging buffer 将下一个暂存缓冲区复制到新的暂存缓冲区
    if (pDX11Texture) {
        m_pDX11DevCtx->CopyResource(pCopyBuffer, pDX11Texture);
    }

    //Create staging buffer for map bits 为映射位创建暂存缓冲区
    IDXGISurface* CopySurface = nullptr;
    hr = pCopyBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&CopySurface);
    if (FAILED(hr)) {
        return false;
    }

    //Copy bits to user space 将位复制到用户空间
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

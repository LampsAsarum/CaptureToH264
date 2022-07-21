#pragma once

// Windows 图形设备接口(GDI)是为与设备无关的图形设计的。基于 Windows 的应用程序不能直接访问图形硬件，
// 应用程序通过 GDI 来与设备驱动程序进行交互。GDI 截图就是通过屏幕的设备环境(DC)获取到当前屏幕的位图数据。

class GDICapture
{
public:
    static bool CaptureRgb24(unsigned char* rgbBuffer, int bufferSize);
    static bool CaptureRgb32(unsigned char* rgbBuffer, int bufferSize);
};


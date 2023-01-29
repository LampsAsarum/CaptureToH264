# CaptureToH264

uses GDI/DXGI to capture the screen, and then uses x264 to convert to H264 format video.

使用 GDI/DXGI 截取屏幕，再使用 x264 转为 H264 格式视频。

## 2022/8/17:

醉了。

本来想将 h264 数据再使用 x264 解码，再渲染。

在网上查资料，发现几乎没有使用 x264 解码的，都是用 FFmpeg，然后看了下 x264 的头文件，好像没有解码的接口。

然后在维基百科（https://zh.m.wikipedia.org/zh-cn/X264）上的看到这句话：x264的主要功能在于进行H.264/MPEG-4 AVC的视频编码，而不是作为解码器（decoder）之用。

嗯，，好吧，我去用 FFmpeg。




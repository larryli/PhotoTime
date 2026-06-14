[English](README.md) | [中文](README_zh.md)

# 🖼 PhotoTime 照片时间修改器

数字照片时间戳管理工具，用于批量处理照片文件的修改时间和 EXIF 时间信息。

[JpgTime](http://www.muralpix.com/jpgtime/) 功能增强。

开发工具：[Pelles C 13.01](http://www.smorgasbordet.com/pellesc/)

## 功能

+ 读取照片文件修改时间、EXIF 时间
+ 从文件名推测时间，支持微信、QQ 等多种格式
+ 自动同步照片时间信息
+ 导出数据为 TSV 或 HTML 文件

## 使用

1. 打开包含照片的文件夹
2. 查看照片时间信息
3. 点击"自动处理"同步时间

支持从资源管理器拖放文件夹到程序窗口。

## 构建

```powershell
cmake -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -B build
cmake --build build --config Release -j
```

## 文档

+ [开发文档](docs/DEVELOPMENT.md)
+ [需求文档](docs/REQUIREMENTS.md)
+ [待办事项](docs/TODO.md)

## 许可证

MIT License

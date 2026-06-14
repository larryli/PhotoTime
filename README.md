[English](README.md) | [中文](README_zh.md)

# 🖼 PhotoTime - Digital Photo Timestamp Manager

Digital photo timestamp manager for batch processing photo file modification times and EXIF time information.

An enhanced version of [JpgTime](http://www.muralpix.com/jpgtime/).

Built with [Pelles C 13.01](http://www.smorgasbordet.com/pellesc/)

## Features

+ Read photo file modification times and EXIF timestamps
+ Extract time from filenames (WeChat, QQ, and other formats)
+ Automatically synchronize photo time information
+ Export data to TSV or HTML files

## Usage

1. Open a folder containing photos
2. View photo time information
3. Click "Autoprocess" to synchronize times

Supports dragging and dropping folders from Explorer.

## Build

```powershell
mkdir build && cd build
cmake ..
cmake --build .
```

## Documentation

+ [Development](docs/DEVELOPMENT.md)
+ [Requirements](docs/REQUIREMENTS.md)
+ [TODO](docs/TODO.md)

## License

MIT License

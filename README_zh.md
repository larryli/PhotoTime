# 🖼 ️PhotoTime 照片时间修改器

[JpgTime](http://www.muralpix.com/jpgtime/) 功能增强。

开发工具：[Pelles C 11.00](http://www.smorgasbordet.com/pellesc/)

## 功能

+ 读取照片文件修改时间、Exif 时间
+ 从文件名推测时间，支持格式：
  - `20060102150405.jpg`
  - `20060102150405.1.jpg`
  - `2006-01-02 15.04.05.jpg`
  - `2006-01-02 15.04.05-1.jpg`
  - `IMG_20060102_150405.jpg`
  - `IMG_20060102_150405_HDR.jpg`
  - `microMsg.1136185445000.jpg`
  - `mmexport1136185445000.jpg`
  - `wx_camera_1136185445000.jpg`
+ 自动处理照片时间
  - 存在 Exif 时间与文件修改时间不一致时将文件修改时间改为 Exif 时间
  - 不存在 Exif 时间时使用文件名推测的时间保存为 Exif 时间，同时设置文件修改时间

## TODO

  - [] 按条件过滤图片
  - [] 按条件选择图片
  - [] 直接设置一个或多个图片的修改时间

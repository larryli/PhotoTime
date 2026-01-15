[English](README.md) | [中文](README_zh.md)

# 🖼 PhotoTime - Digital Photo Timestamp Manager

An enhanced version of [JpgTime](http://www.muralpix.com/jpgtime/) with extended capabilities for managing photo timestamps.

Built with [Pelles C 13.01](http://www.smorgasbordet.com/pellesc/)

## Features

+ Reads photo file modification times and EXIF timestamps
+ Extracts time information from filenames with support for multiple formats:
  - `20060102150405.jpg`
  - `20060102150405.1.jpg`
  - `2006-01-02 15.04.05.jpg`
  - `2006-01-02 15.04.05-1.jpg`
  - `IMG_20060102_150405.jpg`
  - `IMG_20060102_150405_HDR.jpg`
  - `microMsg.1136185445000.jpg`
  - `mmexport1136185445000.jpg`
  - `wx_camera_1136185445000.jpg`
+ Automatically processes photo timestamps:
  - When EXIF time differs from file modification time, updates the file modification time to match the EXIF time
  - When no EXIF time is present, uses time derived from the filename to set both EXIF time and file modification time

## TODO

  - [] Filter images by conditions
  - [] Select images by conditions
  - [] Directly set modification time for one or multiple images

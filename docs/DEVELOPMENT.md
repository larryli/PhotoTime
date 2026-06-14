# PhotoTime 开发文档

## 项目概述

PhotoTime 是一个数字照片时间戳管理工具，用于批量处理照片文件的修改时间和 EXIF 时间信息。

- **开发工具**: Pelles C 13.01
- **构建系统**: CMake 3.8+
- **目标平台**: Windows (Win32 API)
- **字符编码**: Unicode

## 架构概览

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   main.c    │────▶│  photo.c    │────▶│  parsest.c  │
│  (UI层)     │     │ (核心逻辑)  │     │ (时间解析)  │
└─────────────┘     └─────────────┘     └─────────────┘
       │                   │                   │
       │                   │                   │
       ▼                   ▼                   ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│ listview.c  │     │   gdip.c    │     │  utils.c    │
│ (列表视图)  │     │ (图像处理)  │     │ (工具函数)  │
└─────────────┘     └─────────────┘     └─────────────┘
```

### 模块职责

| 模块 | 职责 |
|------|------|
| `main.c` | 程序主入口，窗口创建和消息循环 |
| `photo.c` | 照片处理核心逻辑，时间读取和同步 |
| `parsest.c` | 时间戳解析模块，支持多种文件名格式 |
| `listview.c` | 列表视图控件，显示照片列表 |
| `toolbar.c` | 工具栏控件，提供常用操作按钮 |
| `statusbar.c` | 状态栏控件，显示程序状态信息 |
| `photoview.c` | 照片预览视图 |
| `export.c` | 导出功能 |
| `gdip.c` | GDI+ 图像处理 |
| `about.c` | 关于对话框 |
| `commctrls.c` | Windows 通用控件封装 |
| `utils.c` | 通用工具函数 |

## 编译

### CMake

```powershell
# 创建构建目录
mkdir build
cd build

# 生成构建文件
cmake ..

# 编译项目
cmake --build .

# 编译测试程序
cmake --build . --target parsest
```

输出：`build/PhotoTime.exe`（主程序）或 `build/parsest.exe`（测试程序）

## 项目结构

```
PhotoTime/
├── src/                # 源代码目录
│   ├── main.c          # 主程序入口
│   ├── photo.c         # 照片处理核心逻辑
│   ├── parsest.c       # 时间戳解析模块
│   ├── listview.c      # 列表视图控件
│   ├── toolbar.c       # 工具栏控件
│   ├── statusbar.c     # 状态栏控件
│   ├── photoview.c     # 照片预览视图
│   ├── export.c        # 导出功能
│   ├── gdip.c          # GDI+ 图像处理
│   ├── about.c         # 关于对话框
│   ├── commctrls.c     # 通用控件辅助
│   ├── utils.c         # 工具函数
│   └── res/            # 资源文件
├── CMakeLists.txt      # CMake 构建配置
└── docs/               # 项目文档
```

## 模块划分

### 核心模块

1. **照片处理模块 (photo.c)**
   - 读取照片文件修改时间
   - 读取 EXIF 时间戳
   - 从文件名推测时间
   - 自动同步时间戳

2. **时间戳解析模块 (parsest.c)**
   - 支持多种文件名格式解析
   - 处理微信、QQ 等社交应用导出的照片命名

### 界面模块

3. **主窗口模块 (main.c)**
   - 程序主入口
   - 窗口创建和消息循环

4. **列表视图模块 (listview.c)**
   - 显示照片列表
   - 支持多选、排序

5. **工具栏模块 (toolbar.c)**
   - 提供常用操作按钮

6. **状态栏模块 (statusbar.c)**
   - 显示程序状态信息

7. **照片预览模块 (photoview.c)**
   - 照片预览功能

### 辅助模块

8. **导出模块 (export.c)**
   - 处理结果导出

9. **GDI+ 模块 (gdip.c)**
   - 图像加载和处理

10. **通用控件辅助模块 (commctrls.c)**
    - Windows 通用控件封装

11. **工具函数模块 (utils.c)**
    - 通用工具函数

## 关键设计决策

1. **Unicode 支持**
   - 使用 `-DUNICODE -D_UNICODE` 编译选项
   - 支持中文文件名和路径

2. **Win32 API**
   - 直接使用 Win32 API，无额外框架依赖
   - 使用 GDI+ 进行图像处理

3. **模块化设计**
   - 各功能模块独立，便于维护和扩展
   - 清晰的模块接口定义

4. **测试支持**
   - 提供独立的解析测试程序 (`parsest`)
   - 使用 `TEST_PARSEST` 宏控制测试代码

## API 参考

### photo.h

| 函数 | 说明 |
|------|------|
| `Photo_ReadFileTime(filename)` | 读取文件修改时间 |
| `Photo_ReadExifTime(filename)` | 读取 EXIF 时间戳 |
| `Photo_ParseTimeFromFilename(filename)` | 从文件名解析时间 |
| `Photo_SyncTime(filename)` | 同步时间戳 |
| `Photo_ProcessDirectory(path)` | 处理目录中的所有照片 |

### parsest.h

| 函数 | 说明 |
|------|------|
| `ParseTime_FromString(str, len)` | 从字符串解析时间 |
| `ParseTime_FromFilename(filename)` | 从文件名解析时间 |
| `ParseTime_FormatTime(time, buf, size)` | 格式化时间字符串 |

### utils.h

| 函数 | 说明 |
|------|------|
| `Utils_FileExists(path)` | 检查文件是否存在 |
| `Utils_GetFileSize(path)` | 获取文件大小 |
| `Utils_PathCombine(dst, base, file)` | 合并路径 |

## 测试方法

### 单元测试

```powershell
# 编译测试程序
cd build
cmake --build . --target parsest

# 运行解析测试
./parsest.exe
```

### 集成测试

1. 准备测试照片文件（包含各种命名格式）
2. 运行 PhotoTime 程序
3. 验证时间戳处理结果

## 调试技巧

1. 使用 Visual Studio 或 Pelles C 调试器
2. 关注 `photo.c` 中的时间处理逻辑
3. 检查文件名解析结果是否符合预期

## 编码规范

1. 使用 C99 标准
2. 函数命名：小写字母 + 下划线
3. 常量命名：大写字母 + 下划线
4. 注释使用英文或中文均可
5. 内存管理：使用 Windows Heap API（`HeapAlloc`/`HeapFree`）

## 实现说明

### 时间处理流程

```
读取文件修改时间
    │
    ▼
读取 EXIF 时间戳
    │
    ▼
从文件名解析时间
    │
    ▼
判断时间一致性
    │
    ├─ EXIF 时间存在且与文件时间不一致 → 更新文件时间为 EXIF 时间
    │
    └─ EXIF 时间不存在 → 使用文件名时间设置 EXIF 和文件时间
```

### 支持的文件名格式

| 格式 | 示例 |
|------|------|
| 标准格式 | `20060102150405.jpg` |
| 带序号 | `20060102150405.1.jpg` |
| 分隔符格式 | `2006-01-02 15.04.05.jpg` |
| 分隔符带序号 | `2006-01-02 15.04.05-1.jpg` |
| IMG 前缀 | `IMG_20060102_150405.jpg` |
| IMG 带后缀 | `IMG_20060102_150405_HDR.jpg` |
| 微信格式 | `microMsg.1136185445000.jpg` |
| 微信导出格式 | `mmexport1136185445000.jpg` |
| 微信相机格式 | `wx_camera_1136185445000.jpg` |

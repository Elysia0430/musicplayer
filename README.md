# Music Player

一个使用 C++17 编写的命令行音乐播放器，支持多种音频格式和丰富的播放功能。

```
  __  __           _        ____  _                       
 |  \/  |_   _ ___(_) ___  |  _ \| | __ _ _   _  ___ _ __ 
 | |\/| | | | / __| |/ __| | |_) | |/ _` | | | |/ _ \ '__|
 | |  | | |_| \__ \ | (__  |  __/| | (_| | |_| |  __/ |   
 |_|  |_|\__,_|___/_|\___| |_|   |_|\__,_|\__, |\___|_|   
                                          |___/           
```

## 功能特性

- **播放控制**: 播放、暂停、停止、上一曲、下一曲
- **进度控制**: 跳转到指定位置、快进/快退 10 秒
- **音量控制**: 设置音量 (0-100%)、音量增/减
- **循环模式**: 无循环、单曲循环、列表循环
- **随机播放**: 打乱播放顺序
- **播放列表**: 添加/移除曲目、从目录批量加载、清空列表
- **多格式支持**: MP3, WAV, OGG, FLAC, M4A, WMA

## 音频后端

项目支持两种音频后端：

| 后端 | 平台 | 说明 |
|------|------|------|
| Windows MCI | Windows | 默认后端，使用 Windows 多媒体 API |
| SFML | 跨平台 | 可选后端，需要安装 SFML 库 |

## 编译构建

### 环境要求

- CMake 3.14+
- C++17 兼容的编译器 (MSVC, GCC, Clang)
- Windows MCI 后端: Windows 系统 + winmm 库
- SFML 后端 (可选): SFML 2.5+

### 构建步骤

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目 (使用 Windows MCI 后端, 默认)
cmake ..

# 或使用 SFML 后端
cmake .. -DUSE_SFML=ON -DUSE_WINDOWS=OFF

# 编译
cmake --build .
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `USE_WINDOWS` | ON | 使用 Windows MCI 后端 |
| `USE_SFML` | OFF | 使用 SFML 后端 |

## 使用方法

### 启动播放器

```bash
# 直接启动
./musicplayer

# 启动并加载音频文件
./musicplayer song1.mp3 song2.wav
```

### 命令列表

| 命令 | 简写 | 说明 |
|------|------|------|
| `play` | `p` | 播放/恢复 |
| `pause` | `pa` | 暂停 |
| `stop` | `s` | 停止 |
| `next` | `n` | 下一曲 |
| `prev` | `pr` | 上一曲 |
| `seek <秒>` | - | 跳转到指定位置 |
| `ff` | - | 快进 10 秒 |
| `rw` | - | 快退 10 秒 |
| `vol <0-100>` | - | 设置音量 |
| `vol+` | - | 音量增大 |
| `vol-` | - | 音量减小 |
| `loop` | - | 切换循环模式 (Off/All/Single) |
| `shuffle` | - | 切换随机播放 |
| `add <文件>` | - | 添加文件到播放列表 |
| `load <目录>` | - | 从目录加载所有音频文件 |
| `list` | `ls` | 显示播放列表 |
| `goto <编号>` | - | 跳转到指定曲目 |
| `remove <编号>` | - | 移除指定曲目 |
| `clear` | - | 清空播放列表 |
| `status` | `st` | 显示当前状态 |
| `help` | `h` | 显示帮助 |
| `quit` | `q` | 退出播放器 |

### 使用示例

```bash
> load D:\Music                  # 加载目录中的所有音频文件
Loaded 15 tracks from D:\Music

> list                           # 查看播放列表
=== Playlist ===
 > [1] Song A
   [2] Song B
   [3] Song C

> play                           # 开始播放
Playing...

> vol 80                         # 设置音量为 80%
Volume set to 80%

> loop                           # 切换循环模式
Loop mode: All

> status                         # 查看状态
Now Playing: Song A
Status: Playing | 01:23 / 03:45 | Volume: 80% | Loop: All | Track 1/15
```

## 项目结构

```
.
├── include/
│   ├── AudioPlayer.h          # 音频播放器抽象基类
│   ├── MusicPlayer.h          # 音乐播放器控制器
│   ├── Playlist.h             # 播放列表管理
│   ├── SFMLAudioPlayer.h      # SFML 音频后端实现
│   └── WindowsAudioPlayer.h   # Windows MCI 音频后端实现
├── src/
│   └── main.cpp               # 主程序入口
├── CMakeLists.txt             # CMake 构建配置
└── README.md
```

## 架构设计

项目采用策略模式设计，通过抽象基类 `AudioPlayer` 定义音频播放接口，不同的后端实现（`WindowsAudioPlayer`, `SFMLAudioPlayer`）可以在编译时切换，实现了良好的可扩展性。

```
┌─────────────────┐
│   MusicPlayer   │ ──── 播放器控制器
└────────┬────────┘
         │ 包含
┌────────▼────────┐
│   AudioPlayer   │ ──── 抽象基类
└────────┬────────┘
         │ 实现
    ┌────┴────┐
    ▼         ▼
┌───────┐ ┌───────┐
│Windows│ │ SFML  │ ──── 具体实现
│  MCI  │ │ Audio │
└───────┘ └───────┘
```

## 许可证

MIT License

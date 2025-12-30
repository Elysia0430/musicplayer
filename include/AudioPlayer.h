#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <string>
#include <functional>

namespace MusicApp {

// 播放状态枚举
enum class PlayState {
    Stopped,
    Playing,
    Paused
};

// 循环模式枚举
enum class LoopMode {
    None,       // 不循环
    Single,     // 单曲循环
    All         // 列表循环
};

// 音频播放器抽象基类
class AudioPlayer {
public:
    virtual ~AudioPlayer() = default;

    // 加载音频文件
    virtual bool load(const std::string& filepath) = 0;
    
    // 播放控制
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    
    // 进度控制 (秒)
    virtual void seek(float seconds) = 0;
    virtual float getCurrentTime() const = 0;
    virtual float getDuration() const = 0;
    
    // 音量控制 (0.0 - 100.0)
    virtual void setVolume(float volume) = 0;
    virtual float getVolume() const = 0;
    
    // 状态查询
    virtual PlayState getState() const = 0;
    virtual bool isPlaying() const = 0;
    
    // 获取当前加载的文件路径
    virtual std::string getCurrentFile() const = 0;
    
    // 设置播放结束回调
    using EndCallback = std::function<void()>;
    virtual void setOnEndCallback(EndCallback callback) = 0;
    
    // 更新状态（需要在主循环中调用）
    virtual void update() = 0;
};

} // namespace MusicApp

#endif // AUDIO_PLAYER_H

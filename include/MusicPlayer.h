#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "AudioPlayer.h"
#include "Playlist.h"
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace MusicApp {

// 音乐播放器控制器
class MusicPlayer {
public:
    MusicPlayer(std::unique_ptr<AudioPlayer> player)
        : audioPlayer_(std::move(player)),
          loopMode_(LoopMode::None),
          isRunning_(true) {
        // 设置播放结束回调
        audioPlayer_->setOnEndCallback([this]() {
            onTrackEnd();
        });
    }
    
    // 播放列表操作
    Playlist& getPlaylist() { return playlist_; }
    const Playlist& getPlaylist() const { return playlist_; }
    
    // 添加曲目并开始播放
    bool addAndPlay(const std::string& filepath) {
        playlist_.addTrack(filepath);
        return playCurrentTrack();
    }
    
    // 播放当前曲目
    bool playCurrentTrack() {
        const TrackInfo* track = playlist_.getCurrentTrack();
        if (track) {
            if (audioPlayer_->load(track->filepath)) {
                audioPlayer_->play();
                return true;
            }
        }
        return false;
    }
    
    // 播放/暂停切换
    void togglePlayPause() {
        if (audioPlayer_->isPlaying()) {
            audioPlayer_->pause();
        } else if (audioPlayer_->getState() == PlayState::Paused) {
            audioPlayer_->play();
        } else {
            playCurrentTrack();
        }
    }
    
    // 播放
    void play() {
        if (audioPlayer_->getState() == PlayState::Stopped) {
            playCurrentTrack();
        } else {
            audioPlayer_->play();
        }
    }
    
    // 暂停
    void pause() {
        audioPlayer_->pause();
    }
    
    // 停止
    void stop() {
        audioPlayer_->stop();
    }
    
    // 下一曲
    void next() {
        if (playlist_.next()) {
            playCurrentTrack();
        }
    }
    
    // 上一曲
    void previous() {
        // 如果播放超过3秒，回到开头；否则播放上一曲
        if (audioPlayer_->getCurrentTime() > 3.0f) {
            audioPlayer_->seek(0);
        } else {
            if (playlist_.previous()) {
                playCurrentTrack();
            }
        }
    }
    
    // 跳转到指定曲目
    bool jumpTo(size_t index) {
        if (playlist_.jumpTo(index)) {
            return playCurrentTrack();
        }
        return false;
    }
    
    // 进度控制
    void seek(float seconds) {
        audioPlayer_->seek(seconds);
    }
    
    void seekForward(float seconds = 10.0f) {
        float newPos = audioPlayer_->getCurrentTime() + seconds;
        float duration = audioPlayer_->getDuration();
        if (newPos < duration) {
            audioPlayer_->seek(newPos);
        }
    }
    
    void seekBackward(float seconds = 10.0f) {
        float newPos = audioPlayer_->getCurrentTime() - seconds;
        if (newPos < 0) newPos = 0;
        audioPlayer_->seek(newPos);
    }
    
    // 音量控制
    void setVolume(float volume) {
        audioPlayer_->setVolume(volume);
    }
    
    float getVolume() const {
        return audioPlayer_->getVolume();
    }
    
    void volumeUp(float delta = 5.0f) {
        setVolume(getVolume() + delta);
    }
    
    void volumeDown(float delta = 5.0f) {
        setVolume(getVolume() - delta);
    }
    
    // 循环模式
    void setLoopMode(LoopMode mode) {
        loopMode_ = mode;
    }
    
    LoopMode getLoopMode() const {
        return loopMode_;
    }
    
    void toggleLoopMode() {
        switch (loopMode_) {
            case LoopMode::None:
                loopMode_ = LoopMode::All;
                break;
            case LoopMode::All:
                loopMode_ = LoopMode::Single;
                break;
            case LoopMode::Single:
                loopMode_ = LoopMode::None;
                break;
        }
    }
    
    // 随机播放
    void toggleShuffle() {
        playlist_.setShuffle(!playlist_.isShuffleEnabled());
    }
    
    // 状态查询
    bool isPlaying() const {
        return audioPlayer_->isPlaying();
    }
    
    PlayState getState() const {
        return audioPlayer_->getState();
    }
    
    float getCurrentTime() const {
        return audioPlayer_->getCurrentTime();
    }
    
    float getDuration() const {
        return audioPlayer_->getDuration();
    }
    
    // 更新状态
    void update() {
        audioPlayer_->update();
    }
    
    // 运行状态
    bool isRunning() const { return isRunning_; }
    void quit() { isRunning_ = false; }
    
    // 获取状态信息字符串
    std::string getStatusString() const {
        std::stringstream ss;
        
        const TrackInfo* track = playlist_.getCurrentTrack();
        if (track) {
            ss << "Now Playing: " << track->title << "\n";
        }
        
        // 播放状态
        ss << "Status: ";
        switch (audioPlayer_->getState()) {
            case PlayState::Playing: ss << "Playing"; break;
            case PlayState::Paused: ss << "Paused"; break;
            case PlayState::Stopped: ss << "Stopped"; break;
        }
        
        // 进度
        ss << " | " << formatTime(getCurrentTime()) 
           << " / " << formatTime(getDuration());
        
        // 音量
        ss << " | Volume: " << static_cast<int>(getVolume()) << "%";
        
        // 循环模式
        ss << " | Loop: ";
        switch (loopMode_) {
            case LoopMode::None: ss << "Off"; break;
            case LoopMode::Single: ss << "Single"; break;
            case LoopMode::All: ss << "All"; break;
        }
        
        // 随机
        if (playlist_.isShuffleEnabled()) {
            ss << " | Shuffle: On";
        }
        
        // 播放列表位置
        ss << " | Track " << (playlist_.getCurrentIndex() + 1) 
           << "/" << playlist_.size();
        
        return ss.str();
    }
    
    // 获取播放列表字符串
    std::string getPlaylistString() const {
        std::stringstream ss;
        ss << "\n=== Playlist ===\n";
        
        const auto& tracks = playlist_.getTracks();
        for (size_t i = 0; i < tracks.size(); i++) {
            if (static_cast<int>(i) == playlist_.getCurrentIndex()) {
                ss << " > ";
            } else {
                ss << "   ";
            }
            ss << "[" << (i + 1) << "] " << tracks[i].title << "\n";
        }
        
        if (tracks.empty()) {
            ss << "   (empty)\n";
        }
        
        return ss.str();
    }
    
private:
    void onTrackEnd() {
        switch (loopMode_) {
            case LoopMode::Single:
                // 单曲循环
                audioPlayer_->seek(0);
                audioPlayer_->play();
                break;
            case LoopMode::All:
                // 列表循环
                playlist_.next();
                playCurrentTrack();
                break;
            case LoopMode::None:
                // 不循环，播放下一曲直到列表结束
                if (!playlist_.isAtEnd()) {
                    playlist_.next();
                    playCurrentTrack();
                }
                break;
        }
    }
    
    static std::string formatTime(float seconds) {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << mins 
           << ":" << std::setfill('0') << std::setw(2) << secs;
        return ss.str();
    }
    
    std::unique_ptr<AudioPlayer> audioPlayer_;
    Playlist playlist_;
    LoopMode loopMode_;
    bool isRunning_;
};

} // namespace MusicApp

#endif // MUSIC_PLAYER_H

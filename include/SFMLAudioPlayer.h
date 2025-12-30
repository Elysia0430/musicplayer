#ifndef SFML_AUDIO_PLAYER_H
#define SFML_AUDIO_PLAYER_H

#ifdef USE_SFML  // 只有在启用 SFML 后端时才编译

#include "AudioPlayer.h"
#include <SFML/Audio.hpp>
#include <memory>

namespace MusicApp {

// 基于SFML的音频播放器实现
class SFMLAudioPlayer : public AudioPlayer {
public:
    SFMLAudioPlayer() : volume_(50.0f), state_(PlayState::Stopped) {}
    
    ~SFMLAudioPlayer() override {
        stop();
    }
    
    bool load(const std::string& filepath) override {
        stop();
        currentFile_ = filepath;
        
        if (!music_.openFromFile(filepath)) {
            currentFile_.clear();
            return false;
        }
        
        music_.setVolume(volume_);
        state_ = PlayState::Stopped;
        return true;
    }
    
    void play() override {
        if (!currentFile_.empty()) {
            music_.play();
            state_ = PlayState::Playing;
        }
    }
    
    void pause() override {
        if (state_ == PlayState::Playing) {
            music_.pause();
            state_ = PlayState::Paused;
        }
    }
    
    void stop() override {
        music_.stop();
        state_ = PlayState::Stopped;
    }
    
    void seek(float seconds) override {
        music_.setPlayingOffset(sf::seconds(seconds));
    }
    
    float getCurrentTime() const override {
        return music_.getPlayingOffset().asSeconds();
    }
    
    float getDuration() const override {
        return music_.getDuration().asSeconds();
    }
    
    void setVolume(float volume) override {
        volume_ = std::max(0.0f, std::min(100.0f, volume));
        music_.setVolume(volume_);
    }
    
    float getVolume() const override {
        return volume_;
    }
    
    PlayState getState() const override {
        return state_;
    }
    
    bool isPlaying() const override {
        return state_ == PlayState::Playing && 
               music_.getStatus() == sf::Music::Playing;
    }
    
    std::string getCurrentFile() const override {
        return currentFile_;
    }
    
    void setOnEndCallback(EndCallback callback) override {
        onEndCallback_ = callback;
    }
    
    void update() override {
        // 检查是否播放结束
        if (state_ == PlayState::Playing && 
            music_.getStatus() == sf::Music::Stopped) {
            state_ = PlayState::Stopped;
            if (onEndCallback_) {
                onEndCallback_();
            }
        }
    }
    
private:
    mutable sf::Music music_;
    std::string currentFile_;
    float volume_;
    PlayState state_;
    EndCallback onEndCallback_;
};

} // namespace MusicApp

#endif // USE_SFML

#endif // SFML_AUDIO_PLAYER_H

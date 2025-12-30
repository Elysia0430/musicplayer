#ifndef WINDOWS_AUDIO_PLAYER_H
#define WINDOWS_AUDIO_PLAYER_H

#include "AudioPlayer.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <digitalv.h>

// 仅MSVC支持 #pragma comment 自动链接库
// GCC/MinGW 需要在CMakeLists.txt中通过 target_link_libraries 链接 winmm
#ifdef _MSC_VER
    #pragma comment(lib, "winmm.lib")
#endif

namespace MusicApp {

// 基于Windows MCI的音频播放器实现
class WindowsAudioPlayer : public AudioPlayer {
public:
    WindowsAudioPlayer() 
        : volume_(50.0f), state_(PlayState::Stopped), 
          deviceId_(0), duration_(0.0f) {}
    
    ~WindowsAudioPlayer() override {
        stop();
        closeDevice();
    }
    
    bool load(const std::string& filepath) override {
        stop();
        closeDevice();
        
        currentFile_ = filepath;
        
        // 打开音频文件（使用 ANSI 版本 API，在 MinGW 环境下兼容性更好）
        MCI_OPEN_PARMSA openParms = {};
        openParms.lpstrElementName = filepath.c_str();
        
        DWORD result = mciSendCommandA(0, MCI_OPEN, 
            MCI_OPEN_ELEMENT | MCI_WAIT,
            reinterpret_cast<DWORD_PTR>(&openParms));
        
        if (result != 0) {
            currentFile_.clear();
            return false;
        }
        
        deviceId_ = openParms.wDeviceID;
        
        // 获取时长
        MCI_STATUS_PARMS statusParms = {};
        statusParms.dwItem = MCI_STATUS_LENGTH;
        mciSendCommand(deviceId_, MCI_STATUS, 
            MCI_STATUS_ITEM | MCI_WAIT,
            reinterpret_cast<DWORD_PTR>(&statusParms));
        duration_ = statusParms.dwReturn / 1000.0f;
        
        // 设置音量
        applyVolume();
        
        state_ = PlayState::Stopped;
        return true;
    }
    
    void play() override {
        if (deviceId_ != 0) {
            MCI_PLAY_PARMS playParms = {};
            mciSendCommand(deviceId_, MCI_PLAY, 0,
                reinterpret_cast<DWORD_PTR>(&playParms));
            state_ = PlayState::Playing;
        }
    }
    
    void pause() override {
        if (deviceId_ != 0 && state_ == PlayState::Playing) {
            mciSendCommand(deviceId_, MCI_PAUSE, 0, 0);
            state_ = PlayState::Paused;
        }
    }
    
    void stop() override {
        if (deviceId_ != 0) {
            mciSendCommand(deviceId_, MCI_STOP, 0, 0);
            mciSendCommand(deviceId_, MCI_SEEK, MCI_SEEK_TO_START, 0);
            state_ = PlayState::Stopped;
        }
    }
    
    void seek(float seconds) override {
        if (deviceId_ != 0) {
            bool wasPlaying = (state_ == PlayState::Playing);
            
            MCI_SEEK_PARMS seekParms = {};
            seekParms.dwTo = static_cast<DWORD>(seconds * 1000);
            mciSendCommand(deviceId_, MCI_SEEK, MCI_TO | MCI_WAIT,
                reinterpret_cast<DWORD_PTR>(&seekParms));
            
            if (wasPlaying) {
                play();
            }
        }
    }
    
    float getCurrentTime() const override {
        if (deviceId_ == 0) return 0.0f;
        
        MCI_STATUS_PARMS statusParms = {};
        statusParms.dwItem = MCI_STATUS_POSITION;
        mciSendCommand(deviceId_, MCI_STATUS, 
            MCI_STATUS_ITEM | MCI_WAIT,
            reinterpret_cast<DWORD_PTR>(&statusParms));
        return statusParms.dwReturn / 1000.0f;
    }
    
    float getDuration() const override {
        return duration_;
    }
    
    void setVolume(float volume) override {
        volume_ = std::max(0.0f, std::min(100.0f, volume));
        applyVolume();
    }
    
    float getVolume() const override {
        return volume_;
    }
    
    PlayState getState() const override {
        return state_;
    }
    
    bool isPlaying() const override {
        if (deviceId_ == 0) return false;
        
        MCI_STATUS_PARMS statusParms = {};
        statusParms.dwItem = MCI_STATUS_MODE;
        mciSendCommand(deviceId_, MCI_STATUS, 
            MCI_STATUS_ITEM | MCI_WAIT,
            reinterpret_cast<DWORD_PTR>(&statusParms));
        return statusParms.dwReturn == MCI_MODE_PLAY;
    }
    
    std::string getCurrentFile() const override {
        return currentFile_;
    }
    
    void setOnEndCallback(EndCallback callback) override {
        onEndCallback_ = callback;
    }
    
    void update() override {
        if (state_ == PlayState::Playing && deviceId_ != 0) {
            MCI_STATUS_PARMS statusParms = {};
            statusParms.dwItem = MCI_STATUS_MODE;
            mciSendCommand(deviceId_, MCI_STATUS, 
                MCI_STATUS_ITEM | MCI_WAIT,
                reinterpret_cast<DWORD_PTR>(&statusParms));
            
            if (statusParms.dwReturn == MCI_MODE_STOP) {
                state_ = PlayState::Stopped;
                if (onEndCallback_) {
                    onEndCallback_();
                }
            }
        }
    }
    
private:
    void closeDevice() {
        if (deviceId_ != 0) {
            mciSendCommand(deviceId_, MCI_CLOSE, 0, 0);
            deviceId_ = 0;
        }
    }
    
    void applyVolume() {
        if (deviceId_ != 0) {
            // 使用waveOutSetVolume设置系统音量
            DWORD vol = static_cast<DWORD>((volume_ / 100.0f) * 0xFFFF);
            waveOutSetVolume(0, vol | (vol << 16));
        }
    }
    
    std::string currentFile_;
    float volume_;
    PlayState state_;
    MCIDEVICEID deviceId_;
    float duration_;
    EndCallback onEndCallback_;
};

} // namespace MusicApp

#else
// 非Windows平台的空实现
namespace MusicApp {
class WindowsAudioPlayer : public AudioPlayer {
public:
    bool load(const std::string&) override { return false; }
    void play() override {}
    void pause() override {}
    void stop() override {}
    void seek(float) override {}
    float getCurrentTime() const override { return 0; }
    float getDuration() const override { return 0; }
    void setVolume(float) override {}
    float getVolume() const override { return 0; }
    PlayState getState() const override { return PlayState::Stopped; }
    bool isPlaying() const override { return false; }
    std::string getCurrentFile() const override { return ""; }
    void setOnEndCallback(EndCallback) override {}
    void update() override {}
};
}
#endif

#endif // WINDOWS_AUDIO_PLAYER_H

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <vector>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace MusicApp {

// 从路径提取文件名（不含扩展名）
inline std::string extractFileName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? 
                           path.substr(lastSlash + 1) : path;
    size_t lastDot = filename.find_last_of('.');
    return (lastDot != std::string::npos) ? 
           filename.substr(0, lastDot) : filename;
}

// 获取文件扩展名（小写）
inline std::string getExtension(const std::string& path) {
    size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos) return "";
    std::string ext = path.substr(lastDot);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

// 歌曲信息结构
struct TrackInfo {
    std::string filepath;
    std::string title;
    std::string artist;
    float duration;  // 秒
    
    TrackInfo(const std::string& path = "") 
        : filepath(path), duration(0.0f) {
        // 从文件路径提取标题
        if (!path.empty()) {
            title = extractFileName(path);
        }
    }
};

// 播放列表管理类
class Playlist {
public:
    Playlist() : currentIndex_(-1), shuffleMode_(false) {
        std::random_device rd;
        rng_.seed(rd());
    }
    
    // 添加曲目
    void addTrack(const std::string& filepath) {
        tracks_.emplace_back(filepath);
        shuffledIndices_.push_back(tracks_.size() - 1);
        if (currentIndex_ < 0) {
            currentIndex_ = 0;
        }
    }
    
    // 添加多个曲目
    void addTracks(const std::vector<std::string>& files) {
        for (const auto& f : files) {
            addTrack(f);
        }
    }
    
    // 从目录加载音频文件
    int loadFromDirectory(const std::string& dirPath) {
        int count = 0;
#ifdef _WIN32
        WIN32_FIND_DATAA findData;
        std::string searchPath = dirPath + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::string filename = findData.cFileName;
                    std::string ext = getExtension(filename);
                    // 支持常见音频格式
                    if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || 
                        ext == ".flac" || ext == ".m4a" || ext == ".wma") {
                        std::string fullPath = dirPath + "\\" + filename;
                        addTrack(fullPath);
                        count++;
                    }
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(dirPath.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_REG) {
                    std::string filename = entry->d_name;
                    std::string ext = getExtension(filename);
                    if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || 
                        ext == ".flac" || ext == ".m4a" || ext == ".wma") {
                        std::string fullPath = dirPath + "/" + filename;
                        addTrack(fullPath);
                        count++;
                    }
                }
            }
            closedir(dir);
        }
#endif
        return count;
    }
    
    // 移除曲目
    void removeTrack(size_t index) {
        if (index < tracks_.size()) {
            tracks_.erase(tracks_.begin() + index);
            rebuildShuffleIndices();
            if (currentIndex_ >= static_cast<int>(tracks_.size())) {
                currentIndex_ = tracks_.empty() ? -1 : tracks_.size() - 1;
            }
        }
    }
    
    // 清空列表
    void clear() {
        tracks_.clear();
        shuffledIndices_.clear();
        currentIndex_ = -1;
    }
    
    // 获取当前曲目
    const TrackInfo* getCurrentTrack() const {
        if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(tracks_.size())) {
            int idx = shuffleMode_ ? shuffledIndices_[currentIndex_] : currentIndex_;
            return &tracks_[idx];
        }
        return nullptr;
    }
    
    // 获取指定曲目
    const TrackInfo* getTrack(size_t index) const {
        if (index < tracks_.size()) {
            return &tracks_[index];
        }
        return nullptr;
    }
    
    // 下一曲
    bool next() {
        if (tracks_.empty()) return false;
        currentIndex_ = (currentIndex_ + 1) % tracks_.size();
        return true;
    }
    
    // 上一曲
    bool previous() {
        if (tracks_.empty()) return false;
        currentIndex_ = (currentIndex_ - 1 + tracks_.size()) % tracks_.size();
        return true;
    }
    
    // 跳转到指定曲目
    bool jumpTo(size_t index) {
        if (index < tracks_.size()) {
            currentIndex_ = index;
            return true;
        }
        return false;
    }
    
    // 随机播放模式
    void setShuffle(bool enabled) {
        shuffleMode_ = enabled;
        if (enabled) {
            shuffle();
        }
    }
    
    bool isShuffleEnabled() const { return shuffleMode_; }
    
    // 重新洗牌
    void shuffle() {
        rebuildShuffleIndices();
        std::shuffle(shuffledIndices_.begin(), shuffledIndices_.end(), rng_);
    }
    
    // 获取列表大小
    size_t size() const { return tracks_.size(); }
    bool isEmpty() const { return tracks_.empty(); }
    
    // 获取当前索引
    int getCurrentIndex() const { return currentIndex_; }
    
    // 获取所有曲目
    const std::vector<TrackInfo>& getTracks() const { return tracks_; }
    
    // 检查是否到达列表末尾
    bool isAtEnd() const {
        return currentIndex_ >= static_cast<int>(tracks_.size()) - 1;
    }
    
    // 检查是否在列表开头
    bool isAtBeginning() const {
        return currentIndex_ <= 0;
    }
    
private:
    void rebuildShuffleIndices() {
        shuffledIndices_.clear();
        for (size_t i = 0; i < tracks_.size(); i++) {
            shuffledIndices_.push_back(i);
        }
    }
    
    std::vector<TrackInfo> tracks_;
    std::vector<size_t> shuffledIndices_;
    int currentIndex_;
    bool shuffleMode_;
    std::mt19937 rng_;
};

} // namespace MusicApp

#endif // PLAYLIST_H

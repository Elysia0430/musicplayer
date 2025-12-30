#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <memory>

// 根据平台选择音频后端
#ifdef USE_SFML
    #include "SFMLAudioPlayer.h"
    using AudioPlayerImpl = MusicApp::SFMLAudioPlayer;
#else
    #include "WindowsAudioPlayer.h"
    using AudioPlayerImpl = MusicApp::WindowsAudioPlayer;
#endif

#include "MusicPlayer.h"

using namespace MusicApp;

void printHelp() {
    std::cout << R"(
=== Music Player Commands ===
  play, p          - Play/Resume
  pause, pa        - Pause
  stop, s          - Stop
  next, n          - Next track
  prev, pr         - Previous track
  
  seek <seconds>   - Seek to position
  ff               - Fast forward 10s
  rw               - Rewind 10s
  
  vol <0-100>      - Set volume
  vol+ / vol-      - Volume up/down
  
  loop             - Toggle loop mode (Off/All/Single)
  shuffle          - Toggle shuffle mode
  
  add <file>       - Add file to playlist
  load <directory> - Load all audio files from directory
  list, ls         - Show playlist
  goto <number>    - Jump to track number
  remove <number>  - Remove track from playlist
  clear            - Clear playlist
  
  status, st       - Show current status
  help, h          - Show this help
  quit, q          - Exit player

)";
}

void printBanner() {
    std::cout << R"(
  __  __           _        ____  _                       
 |  \/  |_   _ ___(_) ___  |  _ \| | __ _ _   _  ___ _ __ 
 | |\/| | | | / __| |/ __| | |_) | |/ _` | | | |/ _ \ '__|
 | |  | | |_| \__ \ | (__  |  __/| | (_| | |_| |  __/ |   
 |_|  |_|\__,_|___/_|\___| |_|   |_|\__,_|\__, |\___|_|   
                                          |___/           
                           C++ Music Player v1.0
)" << std::endl;
}

std::vector<std::string> parseCommand(const std::string& input) {
    std::vector<std::string> tokens;
    // 移除可能的\r字符和BOM
    std::string cleanInput = input;
    // 移除\r
    cleanInput.erase(std::remove(cleanInput.begin(), cleanInput.end(), '\r'), cleanInput.end());
    // 移除UTF-8 BOM (0xEF 0xBB 0xBF)
    if (cleanInput.size() >= 3 && 
        (unsigned char)cleanInput[0] == 0xEF && 
        (unsigned char)cleanInput[1] == 0xBB && 
        (unsigned char)cleanInput[2] == 0xBF) {
        cleanInput = cleanInput.substr(3);
    }
    // 移除前导空白
    size_t start = cleanInput.find_first_not_of(" \t");
    if (start != std::string::npos) {
        cleanInput = cleanInput.substr(start);
    }
    
    std::istringstream iss(cleanInput);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void processCommand(MusicPlayer& player, const std::vector<std::string>& args) {
    if (args.empty()) return;
    
    const std::string& cmd = args[0];
    
    if (cmd == "play" || cmd == "p") {
        player.play();
        std::cout << "Playing..." << std::endl;
    }
    else if (cmd == "pause" || cmd == "pa") {
        player.pause();
        std::cout << "Paused" << std::endl;
    }
    else if (cmd == "stop" || cmd == "s") {
        player.stop();
        std::cout << "Stopped" << std::endl;
    }
    else if (cmd == "next" || cmd == "n") {
        player.next();
        std::cout << "Next track" << std::endl;
    }
    else if (cmd == "prev" || cmd == "pr") {
        player.previous();
        std::cout << "Previous track" << std::endl;
    }
    else if (cmd == "seek" && args.size() > 1) {
        float seconds = std::stof(args[1]);
        player.seek(seconds);
        std::cout << "Seeking to " << seconds << "s" << std::endl;
    }
    else if (cmd == "ff") {
        player.seekForward();
        std::cout << "Fast forward 10s" << std::endl;
    }
    else if (cmd == "rw") {
        player.seekBackward();
        std::cout << "Rewind 10s" << std::endl;
    }
    else if (cmd == "vol" && args.size() > 1) {
        float vol = std::stof(args[1]);
        player.setVolume(vol);
        std::cout << "Volume set to " << vol << "%" << std::endl;
    }
    else if (cmd == "vol+") {
        player.volumeUp();
        std::cout << "Volume: " << player.getVolume() << "%" << std::endl;
    }
    else if (cmd == "vol-") {
        player.volumeDown();
        std::cout << "Volume: " << player.getVolume() << "%" << std::endl;
    }
    else if (cmd == "loop") {
        player.toggleLoopMode();
        std::cout << "Loop mode: ";
        switch (player.getLoopMode()) {
            case LoopMode::None: std::cout << "Off"; break;
            case LoopMode::Single: std::cout << "Single"; break;
            case LoopMode::All: std::cout << "All"; break;
        }
        std::cout << std::endl;
    }
    else if (cmd == "shuffle") {
        player.toggleShuffle();
        std::cout << "Shuffle: " 
                  << (player.getPlaylist().isShuffleEnabled() ? "On" : "Off") 
                  << std::endl;
    }
    else if (cmd == "add" && args.size() > 1) {
        // 重新拼接文件路径（可能包含空格）
        std::string filepath;
        for (size_t i = 1; i < args.size(); i++) {
            if (i > 1) filepath += " ";
            filepath += args[i];
        }
        player.getPlaylist().addTrack(filepath);
        std::cout << "Added: " << filepath << std::endl;
    }
    else if (cmd == "load" && args.size() > 1) {
        std::string dirPath;
        for (size_t i = 1; i < args.size(); i++) {
            if (i > 1) dirPath += " ";
            dirPath += args[i];
        }
        int count = player.getPlaylist().loadFromDirectory(dirPath);
        std::cout << "Loaded " << count << " tracks from " << dirPath << std::endl;
    }
    else if (cmd == "list" || cmd == "ls") {
        std::cout << player.getPlaylistString();
    }
    else if (cmd == "goto" && args.size() > 1) {
        size_t index = std::stoul(args[1]) - 1;
        if (player.jumpTo(index)) {
            std::cout << "Jumping to track " << (index + 1) << std::endl;
        } else {
            std::cout << "Invalid track number" << std::endl;
        }
    }
    else if (cmd == "remove" && args.size() > 1) {
        size_t index = std::stoul(args[1]) - 1;
        player.getPlaylist().removeTrack(index);
        std::cout << "Removed track " << (index + 1) << std::endl;
    }
    else if (cmd == "clear") {
        player.getPlaylist().clear();
        player.stop();
        std::cout << "Playlist cleared" << std::endl;
    }
    else if (cmd == "status" || cmd == "st") {
        std::cout << "\n" << player.getStatusString() << "\n" << std::endl;
    }
    else if (cmd == "help" || cmd == "h") {
        printHelp();
    }
    else if (cmd == "quit" || cmd == "q" || cmd == "exit") {
        player.quit();
        std::cout << "Goodbye!" << std::endl;
    }
    else {
        std::cout << "Unknown command: " << cmd << ". Type 'help' for commands." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // 创建音频播放器
    auto audioPlayer = std::make_unique<AudioPlayerImpl>();
    MusicPlayer player(std::move(audioPlayer));
    
    std::cout << "Type 'help' for available commands.\n" << std::endl;
    
    // 如果命令行提供了文件，添加到播放列表
    for (int i = 1; i < argc; i++) {
        player.getPlaylist().addTrack(argv[i]);
        std::cout << "Added: " << argv[i] << std::endl;
    }
    
    // 如果有文件，自动开始播放
    if (argc > 1) {
        player.playCurrentTrack();
    }
    
    // 主循环
    std::string input;
    while (player.isRunning()) {
        std::cout << "> ";
        
        if (!std::getline(std::cin, input)) {
            break;
        }
        
        auto args = parseCommand(input);
        
        try {
            processCommand(player, args);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // 更新播放器状态
        player.update();
    }
    
    return 0;
}

#pragma once

#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>

class AudioManager {
    AudioManager() {};

public:
    static AudioManager &getInstance() {
        static AudioManager instance; // This is the single instance
        return instance;
    }

    AudioManager(const AudioManager &) = delete;
    void operator=(const AudioManager &) = delete;

    struct AudioStream {
        unsigned int handle = 0;
        int instrument = 0;
    };
    std::vector<AudioStream> loadedStreams; // Loaded audio streams

    // Initialize the audio manager
    bool Init();

    // Load and manage audio streams
    void loadStreams(std::vector<std::pair<std::string, int> > &paths);
    void unloadStreams();
    void pauseStreams();
    void playStreams();
    void restartStreams();

    // Audio stream information
    double GetMusicTimePlayed();
    double GetMusicTimeLength();

    // Audio stream control
    void UpdateMusicStream(unsigned int handle);
    void unpauseStreams();
    void SetAudioStreamVolume(unsigned int handle, float volume);
    void BeginPlayback(unsigned int handle);
    void StopPlayback(unsigned int handle);

    // Load and play samples
    void loadSample(const std::string &path, const std::string &name);
    void playSample(const std::string &name, float volume);
    void unloadSample(const std::string &name);

private:
    std::unordered_map<std::string, unsigned int> samples; // Loaded audio samples
};

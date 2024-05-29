#include "game/audio.h"
#include "bass.h"
#include "GLFW/glfw3.h"

#ifdef WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux__
    #include <X11/Xlib.h>
    #define GLFW_EXPOSE_NATIVE_X11
#endif

#include "GLFW/glfw3native.h"
#include <vector>
#include <filesystem>
#include <iostream>

// Error checking macro
#define CHECK_BASS_ERROR() { \
    int errorCode = BASS_ErrorGetCode(); \
    if (errorCode != BASS_OK) { \
        std::cerr << "BASS error " << errorCode << " at line " << __LINE__ << std::endl; \
        return false; \
    } \
}

bool AudioManager::Init() {
#ifdef WIN32
    if (!BASS_Init(-1, 44100, 0, glfwGetWin32Window(glfwGetCurrentContext()), NULL)) {
        CHECK_BASS_ERROR();
    }
#elif __linux__
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        CHECK_BASS_ERROR();
    }
#endif
    return true;
}

void AudioManager::loadStreams(std::vector<std::pair<std::string, int>>& paths) {
    int streams = 0;
    for (auto& path : paths) {
        HSTREAM streamHandle = BASS_StreamCreateFile(false, path.first.c_str(), 0, 0, 0);
        if (streamHandle) {
            AudioManager::loadedStreams.push_back({ streamHandle, path.second });
            if (streams != 0) {
                BASS_ChannelSetLink(loadedStreams[0].handle, loadedStreams[streams].handle);
            }
            streams++;
        } else {
            std::cerr << "Failed to load stream: " << path.first << std::endl;
        }
    }
}

void AudioManager::unloadStreams() {
    if (!loadedStreams.empty()) {
        AudioManager::StopPlayback(AudioManager::loadedStreams[0].handle);
        for (auto& stream : AudioManager::loadedStreams) {
            BASS_StreamFree(stream.handle);
        }
        AudioManager::loadedStreams.clear();
    }
}

void AudioManager::pauseStreams() {
    if (!loadedStreams.empty()) {
        BASS_ChannelPause(loadedStreams[0].handle);
    }
}

void AudioManager::playStreams() {
    if (!loadedStreams.empty()) {
        BASS_ChannelPlay(loadedStreams[0].handle, false);
    }
}

void AudioManager::restartStreams() {
    if (!loadedStreams.empty()) {
        for (auto& stream : loadedStreams) {
            BASS_ChannelSetPosition(stream.handle, 0, BASS_POS_BYTE);
        }
        BASS_ChannelPlay(loadedStreams[0].handle, true);
    }
}

double AudioManager::GetMusicTimePlayed(unsigned int handle) {
    return BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetPosition(handle, BASS_POS_BYTE));
}

double AudioManager::GetMusicTimeLength(unsigned int handle) {
    return BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetLength(handle, BASS_POS_BYTE));
}

void AudioManager::SetAudioStreamVolume(unsigned int handle, float volume) {
    BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, volume);
}

void AudioManager::UpdateMusicStream(unsigned int handle) {
    BASS_ChannelUpdate(handle, 0);
}

void AudioManager::BeginPlayback(unsigned int handle) {
    BASS_ChannelStart(handle);
}

void AudioManager::StopPlayback(unsigned int handle) {
    BASS_ChannelStop(handle);
}

void AudioManager::loadSample(const std::string& path, const std::string& name) {
    HSAMPLE sample = BASS_SampleLoad(false, path.c_str(), 0, 0, 1, 0);
    if (sample) {
        samples[name] = sample;
    } else {
        std::cerr << "Failed to load sample: " << path << std::endl;
    }
}

void AudioManager::playSample(const std::string& name) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        HCHANNEL channel = BASS_SampleGetChannel(it->second, false);
        BASS_ChannelPlay(channel, true);
    } else {
        std::cerr << "Sample not found: " << name << std::endl;
    }
}

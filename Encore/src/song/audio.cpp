#include "audio.h"
#include "bass/bass.h"
#include "bass/bassopus.h"
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

#define CHECK_BASS_ERROR2() { \
    int errorCode = BASS_ErrorGetCode(); \
    if (errorCode != BASS_OK) { \
        std::cerr << "BASS error " << errorCode << " at line " << __LINE__ << std::endl; \
    } \
}

bool AudioManager::Init() {
#ifdef WIN32
    if (!BASS_Init(-1, 44100, 0, glfwGetWin32Window(glfwGetCurrentContext()), NULL)) {
        CHECK_BASS_ERROR();
    }
    BASS_PluginLoad("bassopus", 0);
    CHECK_BASS_ERROR();
#else
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        CHECK_BASS_ERROR();
    }
#ifdef __APPLE__
    BASS_PluginLoad("libbassopus.dylib", 0);
#else
    BASS_PluginLoad("libbassopus.so", 0);
#endif
    CHECK_BASS_ERROR();
#endif

    return true;
}

void AudioManager::loadStreams(std::vector<std::pair<std::string, int>>& paths) {
    int streams = 0;
    for (auto& path : paths) {
        HSTREAM streamHandle = BASS_StreamCreateFile(false, path.first.c_str(), 0, 0, 0);
        if (streamHandle) {
            loadedStreams.push_back({ streamHandle, path.second });
            if (streams != 0) {
                BASS_ChannelSetLink(loadedStreams[0].handle, loadedStreams[streams].handle);
                if (BASS_ChannelFlags(streamHandle, 0, 0) & BASS_SAMPLE_LOOP) // looping is currently enabled
                    BASS_ChannelFlags(streamHandle, 0, BASS_SAMPLE_LOOP); // remove the LOOP flag
            }
            streams++;
        } else {
            CHECK_BASS_ERROR2();
            std::cerr << "Failed to load stream: " << path.first << std::endl;
        }
    }
}

void AudioManager::unloadStreams() {
    if (!loadedStreams.empty()) {
        for (auto& stream : loadedStreams) {
            StopPlayback(stream.handle);
            BASS_StreamFree(stream.handle);
        }
        loadedStreams = {};
        loadedStreams.clear();
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

void AudioManager::unpauseStreams() {
    if (!loadedStreams.empty()) {
        for (auto& stream : loadedStreams) {
            int rewindTimeBytes = BASS_ChannelSeconds2Bytes(stream.handle, 3.0);
            int channelPositionBytes = BASS_ChannelGetPosition(stream.handle, BASS_POS_BYTE);

            int position = channelPositionBytes <= rewindTimeBytes ? 0 : channelPositionBytes - rewindTimeBytes;

            BASS_ChannelSetPosition(stream.handle, position, BASS_POS_BYTE);
        }
        BASS_ChannelPlay(loadedStreams[0].handle, false);
    }
}

double AudioManager::GetMusicTimePlayed(unsigned int handle) {
    return BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetPosition(handle, BASS_POS_BYTE));
    CHECK_BASS_ERROR2();
}

double AudioManager::GetMusicTimeLength(unsigned int handle) {
    return BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetLength(handle, BASS_POS_BYTE));
    CHECK_BASS_ERROR2();
}

void AudioManager::SetAudioStreamVolume(unsigned int handle, float volume) {
    BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, volume);
    CHECK_BASS_ERROR2();
}

void AudioManager::UpdateMusicStream(unsigned int handle) {
    BASS_ChannelUpdate(handle, 0);
    CHECK_BASS_ERROR2();
}

void AudioManager::BeginPlayback(unsigned int handle) {
    CHECK_BASS_ERROR2();
    BASS_ChannelStart(handle);

}

void AudioManager::StopPlayback(unsigned int handle) {
    BASS_ChannelStop(handle);
    CHECK_BASS_ERROR2();
}

void AudioManager::loadSample(const std::string& path, const std::string& name) {
    HSAMPLE sample = BASS_SampleLoad(false, path.c_str(), 0, 0, 1, 0);
    if (sample) {
        samples[name] = sample;
    } else {
        std::cerr << "Failed to load sample: " << path << std::endl;
    }
}

void AudioManager::playSample(const std::string& name, float volume) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        HCHANNEL channel = BASS_SampleGetChannel(it->second, false);
        BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
        BASS_ChannelPlay(channel, true);
    } else {
        std::cerr << "Sample not found: " << name << std::endl;
    }
}

void AudioManager::unloadSample(const std::string& name) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        BASS_SampleFree(it->second);
        samples.erase(it);
    } else {
        std::cerr << "Sample not found: " << name << std::endl;
    }
}

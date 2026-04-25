#include "audio.h"

#include "assets.h"
#include "bass/bass.h"
#include "bass/bassopus.h"
#include "GLFW/glfw3.h"
#include "gameplay/enctime.h"
#include "tracy/Tracy.hpp"


#include <vector>
#include <filesystem>
#include <iostream>

// Error checking macro
#define CHECK_BASS_ERROR()                                                               \
    {                                                                                    \
        int errorCode = BASS_ErrorGetCode();                                             \
        if (errorCode != BASS_OK) {                                                      \
            std::cerr << "BASS error " << errorCode << " at line " << __LINE__           \
                      << std::endl;                                                      \
            return false;                                                                \
        }                                                                                \
    }

#define CHECK_BASS_ERROR2()                                                              \
    {                                                                                    \
        int errorCode = BASS_ErrorGetCode();                                             \
        if (errorCode != BASS_OK) {                                                      \
            std::cerr << "BASS error " << errorCode << " at line " << __LINE__           \
                      << std::endl;                                                      \
        }                                                                                \
    }

bool Encore::AudioManager::Init() {
#ifdef WIN32
    if (!BASS_Init(-1, 44100, 0, glfwGetWin32Window(glfwGetCurrentContext()), NULL)) {
        CHECK_BASS_ERROR();
        return false;
    }
    BASS_PluginLoad("bassopus", 0);
    CHECK_BASS_ERROR();
#else
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        CHECK_BASS_ERROR();
        return false;
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

void Encore::AudioManager::loadStreams(std::vector<std::pair<std::string, int> > &paths) {
    ZoneScoped;
    int streams = 0;
    for (auto &path : paths) {
        HSTREAM streamHandle = BASS_StreamCreateFile(false, path.first.c_str(), 0, 0, 0);
        if (streamHandle) {
            AudioStream audio_stream;
            audio_stream.handle = streamHandle;
            audio_stream.instrument = path.second;
            loadedStreams.push_back(std::move(audio_stream));
            if (streams != 0) {
                BASS_ChannelSetLink(
                    loadedStreams[0].handle, loadedStreams[streams].handle
                );
                if (BASS_ChannelFlags(streamHandle, 0, 0) & BASS_SAMPLE_LOOP) // looping
                                                                              // is
                                                                              // currently
                                                                              // enabled
                    BASS_ChannelFlags(streamHandle, 0, BASS_SAMPLE_LOOP); // remove the
                                                                          // LOOP flag
            }
            float rate = 0;
            BASS_ChannelGetAttribute(loadedStreams[streams].handle, BASS_ATTRIB_FREQ, &rate);
            BASS_ChannelSetAttribute(loadedStreams[streams].handle, BASS_ATTRIB_FREQ, rate*songSpeed*debugSpeed);
            streams++;
        } else {
            CHECK_BASS_ERROR2();
            std::cerr << "Failed to load stream: " << path.first << std::endl;
        }
    }
}

void Encore::AudioManager::unloadStreams() {
    if (!loadedStreams.empty()) {
        for (auto &stream : loadedStreams) {
            StopPlayback(stream.handle);
            BASS_StreamFree(stream.handle);
        }
        loadedStreams = {};
        loadedStreams.clear();
    }
}

void Encore::AudioManager::pauseStreams() const {
    if (!loadedStreams.empty()) {
        for (auto stream : loadedStreams) {
            BASS_ChannelPause(stream.handle);
        }
    }
}

void Encore::AudioManager::playStreams() const {
    if (!loadedStreams.empty()) {
        // for (auto stream : loadedStreams) {
        BASS_ChannelPlay(loadedStreams[0].handle, false);
        //}
    }
}

void Encore::AudioManager::restartStreams() const {
    if (!loadedStreams.empty()) {
        for (auto &stream : loadedStreams) {
            BASS_ChannelSetPosition(stream.handle, 0, BASS_POS_BYTE);
        }
        BASS_ChannelPlay(loadedStreams[0].handle, true);
    }
}
void Encore::AudioManager::seekStreams(double time) const {
    time *= songSpeed;
    if (!loadedStreams.empty()) {
        BASS_ChannelPause(loadedStreams[0].handle);
        for (auto &stream : loadedStreams) {
            int rewindTimeBytes = BASS_ChannelSeconds2Bytes(stream.handle, time);
            BASS_ChannelSetPosition(stream.handle, rewindTimeBytes, BASS_POS_BYTE);
        }
        BASS_ChannelPlay(loadedStreams[0].handle, false);
    }
}

void Encore::AudioManager::unpauseStreams() const {
    if (!loadedStreams.empty()) {
        for (auto &stream : loadedStreams) {
            int rewindTimeBytes = BASS_ChannelSeconds2Bytes(stream.handle, 3.0);
            int channelPositionBytes =
                BASS_ChannelGetPosition(stream.handle, BASS_POS_BYTE);

            int position = channelPositionBytes <= rewindTimeBytes
                ? 0
                : channelPositionBytes - rewindTimeBytes;

            BASS_ChannelSetPosition(stream.handle, position, BASS_POS_BYTE);
        }
        BASS_ChannelPlay(loadedStreams[0].handle, false);
    }
}
void Encore::AudioManager::UpdateAudioStreamVolumes() {
    if (loadedStreams.empty())
        return;
    for (const auto &stream : loadedStreams) {
        BASS_ChannelSetAttribute(stream.handle, BASS_ATTRIB_VOL, stream.volume);
    };
    CHECK_BASS_ERROR2();
}
Encore::AudioManager::AudioStream *
Encore::AudioManager::GetAudioStreamByInstrument(int instrument) {
    if (loadedStreams.empty())
        return nullptr;
    for (auto &stream : loadedStreams) {
        if (stream.instrument == instrument)
            return &stream;
    };
    return nullptr;
}

double Encore::AudioManager::GetMusicTimePlayed() const {
    if (loadedStreams.empty()) {
        return 0.0;
    }
    return BASS_ChannelBytes2Seconds(
        loadedStreams[0].handle,
        BASS_ChannelGetPosition(loadedStreams[0].handle, BASS_POS_BYTE)
    ) / songSpeed;
    CHECK_BASS_ERROR2();
}

double Encore::AudioManager::GetMusicTimeLength() const {
    if (loadedStreams.empty()) {
        return 0.0;
    }
    return BASS_ChannelBytes2Seconds(
        loadedStreams[0].handle,
        BASS_ChannelGetLength(loadedStreams[0].handle, BASS_POS_BYTE)
    ) / songSpeed;
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::SetAudioStreamVolume(unsigned int handle, float volume) {
    BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, volume);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::UpdateMusicStream(unsigned int handle) {
    BASS_ChannelUpdate(handle, 0);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::BeginPlayback(unsigned int handle) {
    BASS_ChannelStart(handle);
    CHECK_BASS_ERROR2();
}


void Encore::AudioManager::StartEffect(AudioStream* stream) {
    BASS_DX8_FLANGER effect;
    effect.fWetDryMix = 100;
    BASS_FXSetParameters(BASS_FX_DX8_FLANGER, &effect);
    if (stream)
        stream->fxhandle = BASS_ChannelSetFX(stream->handle, BASS_FX_DX8_FLANGER, 0);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::StopEffect(AudioStream* stream) {
    if (stream)
        BASS_ChannelRemoveFX(stream->handle, stream->fxhandle);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::StopPlayback(unsigned int handle) {
    BASS_ChannelStop(handle);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::SetAudioStreamPosition(unsigned int handle, double time) {
    int positionBytes = BASS_ChannelSeconds2Bytes(handle, time);
    BASS_ChannelSetPosition(handle, positionBytes, BASS_POS_BYTE);
    CHECK_BASS_ERROR2();
}

void Encore::AudioManager::playSample(HSAMPLE sample, float volume) {
    HCHANNEL channel = BASS_SampleGetChannel(sample, false);
    CHECK_BASS_ERROR2();
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
    CHECK_BASS_ERROR2();
    BASS_ChannelPlay(channel, true);
    CHECK_BASS_ERROR2();
}

// Defined here to avoid conflicts with Raylib and BASS
void SampleAsset::Load() {
    LoadFile();
    sample = BASS_SampleLoad(true, fileBuffer, 0, fileSize, MAX_SAMPLE_CHANNELS, 0);
    state = LOADED;
}
void SampleAsset::Unload() {
    BASS_SampleFree(sample);
    sample = 0;
}

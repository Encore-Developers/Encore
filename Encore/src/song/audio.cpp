#include "audio.h"

#include "bass/bass.h"
#include "bass/bassopus.h"
#include "tracy/Tracy.hpp"

#include <vector>
#include <filesystem>
#include <iostream>

#include "util/enclog.h"

std::string BASSErrNoToString(int err) {
    switch (err) {
        case -1: return "BASS_ERROR_UNKNOWN";
        case 0:  return "BASS_OK";
        case 1:  return "BASS_ERROR_MEM";
        case 2:	 return "BASS_ERROR_FILEOPEN";
        case 3:	 return "BASS_ERROR_DRIVER";
        case 4:	 return "BASS_ERROR_BUFLOST";
        case 5:	 return "BASS_ERROR_HANDLE";
        case 6:	 return "BASS_ERROR_FORMAT";
        case 7:	 return "BASS_ERROR_POSITION";
        case 8:	 return "BASS_ERROR_INIT";
        case 9:	 return "BASS_ERROR_START";
        case 10: return "BASS_ERROR_SSL";
        case 11: return "BASS_ERROR_REINIT";
        case 13: return "BASS_ERROR_TRACK";
        case 14: return "BASS_ERROR_ALREADY";
        case 17: return "BASS_ERROR_NOTAUDIO";
        case 18: return "BASS_ERROR_NOCHAN";
        case 19: return "BASS_ERROR_ILLTYPE";
        case 20: return "BASS_ERROR_ILLPARAM";
        case 21: return "BASS_ERROR_NO3D";
        case 22: return "BASS_ERROR_NOEAX";
        case 23: return "BASS_ERROR_DEVICE";
        case 24: return "BASS_ERROR_NOPLAY";
        case 25: return "BASS_ERROR_FREQ";
        case 27: return "BASS_ERROR_NOTFILE";
        case 29: return "BASS_ERROR_NOHW";
        case 31: return "BASS_ERROR_EMPTY";
        case 32: return "BASS_ERROR_NONET";
        case 33: return "BASS_ERROR_CREATE";
        case 34: return "BASS_ERROR_NOFX";
        case 37: return "BASS_ERROR_NOTAVAIL";
        case 38: return "BASS_ERROR_DECODE";
        case 39: return "BASS_ERROR_DX";
        case 40: return "BASS_ERROR_TIMEOUT";
        case 41: return "BASS_ERROR_FILEFORM";
        case 42: return "BASS_ERROR_SPEAKER";
        case 43: return "BASS_ERROR_VERSION";
        case 44: return "BASS_ERROR_CODEC";
        case 45: return "BASS_ERROR_ENDED";
        case 46: return "BASS_ERROR_BUSY";
        case 47: return "BASS_ERROR_UNSTREAMABLE";
        case 48: return "BASS_ERROR_PROTOCOL";
        case 49: return "BASS_ERROR_DENIED";
        case 50: return "BASS_ERROR_FREEING";
        case 51: return "BASS_ERROR_CANCEL";
    }
}

// Error checking macro
#define CHECK_BASS_ERROR_FMT(what, ...)                                                              \
                                                                                        \
    int errorCode = BASS_ErrorGetCode();                                             \
    if (errorCode != BASS_OK) {                                                      \
        Encore::Log::Error("BASS error {} in audio.cpp func {}, line {}", BASSErrNoToString(errorCode), __FUNCTION__, __LINE__);           \
        Encore::Log::Error(what, std::make_format_args(__VA_ARGS__));                                                                 \
    }                                                                                \

#define CHECK_BASS_ERROR(what)                                                              \
\
int errorCode = BASS_ErrorGetCode();                                             \
if (errorCode != BASS_OK) {                                                      \
Encore::Log::Error("BASS error {} in audio.cpp func {}, line {}", BASSErrNoToString(errorCode), __FUNCTION__, __LINE__);           \
Encore::Log::Error(what);                                                                 \
}                                                                                \


bool Encore::AudioManager::Init() {
#ifdef WIN32
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {

        CHECK_BASS_ERROR("Couldn't initialize BASS");
        return false;
    }
    BASS_PluginLoad("bassopus", 0);
    CHECK_BASS_ERROR("Couldn't initialize BASSOPUS");
#else
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        CHECK_BASS_ERROR("Couldn't initialize BASS");
        return false;
    }
#ifdef __APPLE__
    BASS_PluginLoad("libbassopus.dylib", 0);
#else
    BASS_PluginLoad("libbassopus.so", 0);
#endif
    CHECK_BASS_ERROR("Couldn't initialize BASSOPUS");
#endif
    if (errorCode != BASS_OK) {return false;}
    Log::Debug("BASS Initialized");
    return true;
}

void Encore::AudioManager::loadStreams(std::vector<std::pair<std::filesystem::path, Stems>> paths) {
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
            std::string pString = path.first.string();
            std::string what = "Failed to load stream: {}";
            // i hate you
            int errorCode = BASS_ErrorGetCode();
            if (errorCode != BASS_OK) {
                Log::Error("BASS error {} in audio.cpp func {}, line {}", BASSErrNoToString(errorCode), __FUNCTION__, __LINE__);
                Log::Error(what, pString);
            };
        }
    }
}

void Encore::AudioManager::unloadStreams() {
    if (!loadedStreams.empty()) {
        for (auto &stream : loadedStreams) {
            StopPlayback(stream.handle);
            BASS_StreamFree(stream.handle);
        }
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
    CHECK_BASS_ERROR("Failed to update stream volume");
}

Encore::AudioManager::AudioStream *
Encore::AudioManager::GetAudioStreamByInstrument(Stems stem) {
    if (loadedStreams.empty())
        return nullptr;
    for (auto &stream : loadedStreams) {
        if (stream.instrument == stem)
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
    CHECK_BASS_ERROR("Failed to get music time played");
}

double Encore::AudioManager::GetMusicTimeLength() const {
    if (loadedStreams.empty()) {
        return 0.0;
    }
    return BASS_ChannelBytes2Seconds(
        loadedStreams[0].handle,
        BASS_ChannelGetLength(loadedStreams[0].handle, BASS_POS_BYTE)
    ) / songSpeed;
    CHECK_BASS_ERROR("Failed to music length");
}

void Encore::AudioManager::SetAudioStreamVolume(Stems stem, float volume) {
    for (auto &stream : loadedStreams) {
        if (stem == stream.instrument)
            stream.volume = volume;
    }
    CHECK_BASS_ERROR("Failed to set stream volume");
}

void Encore::AudioManager::UpdateMusicStream(unsigned int handle) {
    BASS_ChannelUpdate(handle, 0);
    CHECK_BASS_ERROR("Failed to update music stream");
}

void Encore::AudioManager::BeginPlayback(unsigned int handle) {
    BASS_ChannelStart(handle);
    CHECK_BASS_ERROR("Failed to begin playback");
}


void Encore::AudioManager::StartEffect(AudioStream* stream) {
    BASS_DX8_FLANGER effect;
    effect.fWetDryMix = 100;
    BASS_FXSetParameters(BASS_FX_DX8_FLANGER, &effect);
    if (stream)
        stream->fxhandle = BASS_ChannelSetFX(stream->handle, BASS_FX_DX8_FLANGER, 0);
    CHECK_BASS_ERROR("Failed to start channel effects");
}

void Encore::AudioManager::StopEffect(AudioStream* stream) {
    if (stream)
        BASS_ChannelRemoveFX(stream->handle, stream->fxhandle);
    CHECK_BASS_ERROR("Failed to stop channel effects");
}

void Encore::AudioManager::StopPlayback(unsigned int handle) {
    BASS_ChannelStop(handle);
    CHECK_BASS_ERROR("Failed to stop playback");
}

void Encore::AudioManager::SetAudioStreamPosition(unsigned int handle, double time) {
    int positionBytes = BASS_ChannelSeconds2Bytes(handle, time);
    BASS_ChannelSetPosition(handle, positionBytes, BASS_POS_BYTE);
    CHECK_BASS_ERROR("Failed to set audio stream position");
}


void Encore::AudioManager::playSample(unsigned long sample, float volume) {
    // see no evil, hear no evil, speak no evil
    HCHANNEL channel = BASS_SampleGetChannel(sample, false);
    {
        CHECK_BASS_ERROR("Failed to get SFX channel");
    }
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
    {

        CHECK_BASS_ERROR("Failed to set SFX channel volume");
    }
    BASS_ChannelPlay(channel, true);
    {
        CHECK_BASS_ERROR("Failed to play SFX channel");
    }
}

unsigned long Encore::AudioManager::loadSample(bool mem, void *file, size_t length) {
    return BASS_SampleLoad(mem, file, 0, length, MAX_SAMPLE_CHANNELS, 0);
}

void Encore::AudioManager::unloadSample(unsigned long sample) {
    BASS_SampleFree(sample);
}

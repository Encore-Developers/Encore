#include "audio.h"

#include "GLFW/glfw3.h"
#include "gameplay/enctime.h"
#include "tracy/Tracy.hpp"

#include "GLFW/glfw3native.h"
#include <vector>
#include <filesystem>
#include <iostream>

#define CHECK_SDL_ERROR(value, failreturn) if (!value) {\
    std::cerr << "SDL error: " << SDL_GetError() << " at line " << __LINE__ << std::endl; \
    return failreturn; \
}

Encore::AudioManager::AudioStream::AudioStream(const std::string &path, int instrument) {
    track = MIX_CreateTrack(TheAudioManager.mixer);
    CHECK_SDL_ERROR(track, )

    audio = MIX_LoadAudio(TheAudioManager.mixer, path.c_str(), false);
    CHECK_SDL_ERROR(audio, )

    MIX_TagTrack(track, "stem");
    MIX_SetTrackAudio(track, audio);
}
void Encore::AudioManager::AudioStream::SetVolume(float volume) {
    MIX_SetTrackGain(track, volume);
    this->volume = volume;
}
void Encore::AudioManager::AudioStream::Seek(double time) const {
    MIX_SetTrackPlaybackPosition(track, MIX_TrackMSToFrames(track, time*1000));
}
Encore::AudioManager::AudioStream::~AudioStream() {
    MIX_DestroyTrack(track);
    MIX_DestroyAudio(audio);
}

bool Encore::AudioManager::Init() {
    CHECK_SDL_ERROR(MIX_Init(), false)
    TheAudioManager.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    CHECK_SDL_ERROR(TheAudioManager.mixer, false)
    return true;
}

void Encore::AudioManager::loadStreams(std::vector<std::pair<std::string, int>> paths) {
    ZoneScoped;
    int streams = 0;
    for (auto &path : paths) {
        auto stream = std::make_shared<AudioStream>(path.first, path.second);
        loadedStreams.push_back(stream);
        //audio_stream.instrument = path.second;
    }
}

void Encore::AudioManager::unloadStreams() {
    loadedStreams.clear();

}

void Encore::AudioManager::pauseStreams() const {
    MIX_PauseTag(mixer, "stem");
}

void Encore::AudioManager::playStreams(double time) const {
    static auto props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_START_MILLISECOND_NUMBER, time*1000);
    MIX_PlayTag(mixer, "stem", props);
}

void Encore::AudioManager::restartStreams() const {
    seekStreams(0);

}
void Encore::AudioManager::seekStreams(double time) const {
    time *= songSpeed;
    MIX_PauseTag(mixer, "stem");
    for (auto& stream : loadedStreams) {
        stream->Seek(time);
    }
    MIX_ResumeTag(mixer, "stem");
}

void Encore::AudioManager::unpauseStreams() const {
    MIX_ResumeTag(mixer, "stem");
}
void Encore::AudioManager::UpdateAudioStreamVolumes() {

}
Encore::AudioManager::AudioStream *
Encore::AudioManager::GetAudioStreamByInstrument(int instrument) {
    if (loadedStreams.empty())
        return nullptr;
    for (auto &stream : loadedStreams) {
        if (stream->instrument == instrument)
            return stream.get();
    };
    return nullptr;
}

double Encore::AudioManager::GetMusicTimePlayed() const {
    if (loadedStreams.empty()) {
        return 0.0;
    }
    // divide by song speed
    return (MIX_TrackFramesToMS(loadedStreams[0]->track, (double)MIX_GetTrackPlaybackPosition(loadedStreams[0]->track))/1000.0)/songSpeed;
}

double Encore::AudioManager::GetMusicTimeLength() const {
    if (loadedStreams.empty()) {
        return 0.0;
    }
    // divide by song speed
    return (MIX_AudioFramesToMS(loadedStreams[0]->audio, (double)MIX_GetAudioDuration(loadedStreams[0]->audio))/1000.0f)/songSpeed;
}


void Encore::AudioManager::StartEffect(AudioStream* stream) {

}

void Encore::AudioManager::StopEffect(AudioStream* stream) {

}

void Encore::AudioManager::playSample(unsigned long sample, float volume) {

}

unsigned long Encore::AudioManager::loadSample(bool mem, void *file, size_t length) {
    return 0;
}

void Encore::AudioManager::unloadSample(unsigned long sample) {

}

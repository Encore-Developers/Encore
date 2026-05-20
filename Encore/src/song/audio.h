#pragma once

#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>
#include "SDL3_mixer/SDL_mixer.h"

// change this to allow a sound effect to repeat more
#define MAX_SAMPLE_CHANNELS 8

namespace Encore {
    class AudioManager {
    public:

        MIX_Mixer* mixer = 0;
        class AudioStream {
        public:
            MIX_Track* track = 0;
            MIX_Audio* audio = 0;
            float volume = 1.0;
            int instrument = 0;
            unsigned long fxhandle = 0;

            AudioStream(const std::string& path, int instrument);
            AudioStream(const AudioStream&) = delete;

            void SetVolume(float volume);
            void Seek(double time) const;

            ~AudioStream();
        };
        std::vector<std::shared_ptr<AudioStream>> loadedStreams; // Loaded audio streams

        float songSpeed = 1.0;
        float debugSpeed = 1.0;

        // Initialize the audio manager
        static bool Init();

        // Load and manage audio streams
        void loadStreams(std::vector<std::pair<std::string, int> > paths);
        void unloadStreams();
        void pauseStreams() const;
        void playStreams(double time = 0) const;
        void restartStreams() const;
        void seekStreams(double time) const;
        void stopStreams() const;
        // Audio stream information
        double GetMusicTimePlayed() const;
        [[nodiscard]] double GetMusicTimeLength() const;

        // Audio stream control
        void unpauseStreams() const;
        void UpdateAudioStreamVolumes();
        AudioStream* GetAudioStreamByInstrument(int instrument);

        // Load and play samples
        void playSample(unsigned long sample, float volume);
        unsigned long loadSample(bool mem, void *file, size_t length);
        void unloadSample(unsigned long sample);
        //
        void StartEffect(AudioStream* stream);
        void StopEffect(AudioStream* stream);

    private:
        std::unordered_map<std::string, unsigned int> samples; // Loaded audio samples
    };
}
extern Encore::AudioManager TheAudioManager;
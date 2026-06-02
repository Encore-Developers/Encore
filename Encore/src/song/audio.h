#pragma once

#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>

// change this to allow a sound effect to repeat more
#define MAX_SAMPLE_CHANNELS 8

#define AUDIOSTEM(stem) Encore::AudioManager::Stems::stem

namespace Encore {
    class AudioManager {
    public:
        enum class Stems {
            Drums1 = 0, // drums_1, drums
            Drums2, // drums_2
            Drums3, // drums_3
            Drums4, // drums_4
            Bass, // bass, rhythm
            Guitar, // guitar
            Keys, // keys
            Vocals, // vocals_1, vocals
            BackingVocals, // vocals_2
            Background, // song
            Crowd, // crowd
            STEMS_MAX
        };

        struct AudioStream {
            unsigned int handle = 0;
            float volume = 1.0;
            Stems instrument;
            unsigned long fxhandle = 0;
        };
        std::vector<AudioStream> loadedStreams; // Loaded audio streams

        float songSpeed = 1.0;
        float debugSpeed = 1.0;

        // Initialize the audio manager
        static bool Init();

        // Load and manage audio streams
        void loadStreams(std::vector<std::pair<std::filesystem::path, Stems> > paths);
        void unloadStreams();
        void pauseStreams() const;
        void playStreams() const;
        void restartStreams() const;
        void seekStreams(double time) const;
        // Audio stream information
        double GetMusicTimePlayed() const;
        [[nodiscard]] double GetMusicTimeLength() const;

        // Audio stream control
        static void UpdateMusicStream(unsigned int handle);
        void unpauseStreams() const;
        void UpdateAudioStreamVolumes();
        AudioStream* GetAudioStreamByInstrument(Stems stem);
        void SetAudioStreamVolume(Stems stem, float volume);
        static void SetAudioStreamPosition(unsigned int handle, double time);
        static void BeginPlayback(unsigned int handle);
        static void StopPlayback(unsigned int handle);

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
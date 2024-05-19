#pragma once
#include <vector>
#include <filesystem>
#include <unordered_map>
class AudioManager {
public:
	struct audioStream {
		unsigned int handle = 0;
		int instrument = 0;
	};
	bool Init();
	std::vector<audioStream> loadedStreams;
	void loadStreams(std::vector<std::pair<std::string, int>>&);
	void unloadStreams();
	double GetMusicTimePlayed(unsigned int);
	double GetMusicTimeLength(unsigned int);
	void UpdateMusicStream(unsigned int);
	void SetAudioStreamVolume(unsigned int, float);
	void BeginPlayback(unsigned int);
	void StopPlayback(unsigned int);

    void pauseStreams();
    void playStreams();
    void restartStreams();
	
    void loadSample(const std::string& path, const std::string& name);
    void playSample(const std::string& name);

private:
    std::unordered_map<std::string, unsigned int> samples;
};
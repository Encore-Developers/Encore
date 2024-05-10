#pragma once
#include <vector>
#include <filesystem>
class AudioManager {
public:
	bool Init();
	std::vector<std::pair<unsigned int, int>> loadedStreams;
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
};
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
bool AudioManager::Init() {
#ifdef WIN32
	return BASS_Init(-1, 44100, 0, glfwGetWin32Window(glfwGetCurrentContext()), NULL);
#elif __linux__
	return BASS_Init(-1, 44100, 0, glfwGetX11Window(glfwGetCurrentContext()), NULL);
#endif
};
void AudioManager::loadStreams(std::vector<std::pair<std::string,int>>& paths) {
	for (auto& path : paths) {
		AudioManager::loadedStreams.push_back({ BASS_StreamCreateFile(false, path.first.c_str(), 0, 0, 0),path.second });
	}
}
void AudioManager::unloadStreams() {
	for (auto& stream : AudioManager::loadedStreams) {
		AudioManager::StopPlayback(stream.first);
		BASS_StreamFree(stream.first);
	}
	AudioManager::loadedStreams.clear();
}
double AudioManager::GetMusicTimePlayed(unsigned int handle) {
	return BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetPosition(handle,BASS_POS_BYTE));
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
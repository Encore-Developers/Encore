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
	return BASS_Init(-1, 44100, 0, 0, NULL);
#endif
};

void AudioManager::loadStreams(std::vector<std::pair<std::string,int>>& paths) {
    int streams = 0;
	for (auto& path : paths) {
		AudioManager::loadedStreams.push_back({ BASS_StreamCreateFile(false, path.first.c_str(), 0, 0, 0),path.second });
	    if (streams != 0) {
            BASS_ChannelSetLink(loadedStreams[0].handle, loadedStreams[streams].handle);
        }
        streams += 1;
    }

}
void AudioManager::unloadStreams() {
    AudioManager::StopPlayback(AudioManager::loadedStreams[0].handle);
	for (auto& stream : AudioManager::loadedStreams) {
		BASS_StreamFree(stream.handle);
	}
	AudioManager::loadedStreams.clear();
}
void AudioManager::pauseStreams() {
	BASS_ChannelPause(loadedStreams[0].handle);

}
void AudioManager::playStreams() {
    BASS_ChannelPlay(loadedStreams[0].handle,false);
}
void AudioManager::restartStreams() {
    for (auto& stream : AudioManager::loadedStreams) {
        BASS_ChannelSetPosition(stream.handle, 0, BASS_POS_BYTE);
    }
	BASS_ChannelPlay(loadedStreams[0].handle, true);
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
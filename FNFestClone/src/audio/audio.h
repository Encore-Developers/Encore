#pragma once
#include "raylib.h"

#include <vector>
#include <string>

bool AudioInit() {
	InitAudioDevice();
	return IsAudioDeviceReady();
}

std::vector<Music> LoadStems(std::vector<std::string> paths) {
	std::vector<Music> loadedStreams;
	for (std::string& path : paths) {
		if (path != "") {
			loadedStreams.push_back(LoadMusicStream(path.c_str()));
		}
		
	}
	return loadedStreams;
}
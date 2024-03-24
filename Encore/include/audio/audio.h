#pragma once
#include "raylib.h"

#include <vector>
#include <string>

bool AudioInit() 
{
	InitAudioDevice();
	return IsAudioDeviceReady();
}

std::vector<std::pair<Music,int>> LoadStems(std::vector<std::pair<std::string,int>> paths) 
{
	std::vector<std::pair<Music,int>> loadedStreams;
	for (auto& path : paths) 
	{
		if (path.first != "") 
		{
			
			loadedStreams.push_back({ LoadMusicStream(path.first.c_str()),path.second });
		}
		
	}
	return loadedStreams;
}
//
// Created by marie on 08/08/2024.
//

#include "settings.h"

#include <fstream>
#include <iostream>

#include "rapidjson/filewritestream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

void Settings::SaveSettings (const std::filesystem::path& settingsFile) {
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	for (const auto& pair : settings) {
		doc.AddMember(rapidjson::Value(pair.first.c_str(), allocator), pair.second->toJson(allocator), allocator);
	}

	std::ofstream ofs(settingsFile);
	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	doc.Accept(writer);
}

void Settings::LoadSettings(const std::filesystem::path& settingsFile) {
	std::ifstream ifs(settingsFile);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);

	if (doc.HasParseError()) {
		std::cerr << "Error parsing settings file." << std::endl;
		return;
	}

	for (auto& m : doc.GetObject()) {
		if (settings.find(m.name.GetString()) != settings.end()) {
			settings[m.name.GetString()]->SetValue(m.value);
		}
	}
}

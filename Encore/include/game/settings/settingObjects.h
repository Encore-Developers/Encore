#ifndef SETTINGOBJECTS_H
#define SETTINGOBJECTS_H
#include <vector>

#include "rapidjson/document.h"

class SettingObject {
public:
	virtual ~SettingObject() = default;
	virtual void SetValue(const rapidjson::Value& value) = 0;
	virtual rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const = 0;
};

class IntSetting : public SettingObject {
	int value;
public:
	IntSetting(int defaultValue) : value(defaultValue) {}
	void SetValue(const rapidjson::Value& value) override {
		if (value.IsInt()) {
			this->value = value.GetInt();
		}
	}

	rapidjson::Value toJson(rapidjson::Document::AllocatorType &allocator) const override {
		return rapidjson::Value(value);
	}
};

class FloatSetting : public SettingObject {
	float value;
public:
	FloatSetting(int defaultValue) : value(defaultValue) {}
	void SetValue(const rapidjson::Value& value) override {
		if (value.IsFloat()) {
			this->value = value.GetFloat();
		}
	}
	rapidjson::Value toJson(rapidjson::Document::AllocatorType &allocator) const override {
		return rapidjson::Value(value);
	}
};

class BoolSetting : public SettingObject {
	bool value;
public:
	BoolSetting(bool defaultValue) : value(defaultValue) {}

	void SetValue(const rapidjson::Value& value) override {
		if (value.IsBool()) {
			this->value = value.GetBool();
		}
	}
	rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const override {
		return rapidjson::Value(value);
	}
};

template <typename t>
class ArraySetting : public SettingObject {
	std::vector<t> value;
public:
	ArraySetting(const std::vector<t>& defaultValue) : value(defaultValue) {}

	void SetValue(const rapidjson::Value& value) override {
		if (value.IsArray()) {
			this->value.clear();
			for (auto& v : value.GetArray()) {
				if (v.Is<t>()) {
					this->value.push_back(v.Get<t>());
				}
			}
		}
	}
	rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const override {
		rapidjson::Value array(rapidjson::kArrayType);
		for (const auto& v : value) {
			array.PushBack(v, allocator);
		}
		return array;
	}
	const std::vector<t>& getValue() const {
		return value;
	}
};

// Specialization for std::string
template <>
inline void ArraySetting<std::string>::SetValue(const rapidjson::Value& value) {
	if (value.IsArray()) {
		this->value.clear();
		for (auto& v : value.GetArray()) {
			if (v.IsString()) {
				this->value.push_back(v.GetString());
			}
		}
	}
}

template <>
inline rapidjson::Value ArraySetting<std::string>::toJson(rapidjson::Document::AllocatorType& allocator) const {
	rapidjson::Value array(rapidjson::kArrayType);
	for (const auto& v : value) {
		array.PushBack(rapidjson::Value(v.c_str(), allocator).Move(), allocator);
	}
	return array;
}

#endif // SETTINGOBJECTS_H

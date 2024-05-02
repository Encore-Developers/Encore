#pragma once
#include <vector>
#include <string>

using namespace std;

//https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
std::vector<std::string> split(const std::string& s, char seperator)
{
	std::vector<std::string> output;

	std::string::size_type prev_pos = 0, pos = 0;

	while ((pos = s.find(seperator, pos)) != std::string::npos)
	{
		std::string substring(s.substr(prev_pos, pos - prev_pos));

		output.push_back(substring);

		prev_pos = ++pos;
	}

	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

	return output;
}

class ArgumentList
{
public:
	static vector<string> arguments;

	static void InitArguments(int ArgCount, char** Args)
	{
		arguments = vector<string>(Args + 1, Args + ArgCount);
	}

	//arg format: key=value
	static string GetArgValue(string key)
	{
		for (auto arg : arguments)
		{
			vector<string> keyVal = split(arg, '=');

			if (keyVal[0] == key)
			{
				return keyVal[1];
			}
		}

		return "";
	}
};
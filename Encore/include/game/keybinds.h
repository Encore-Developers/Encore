#pragma once
#include "raylib.h"
#include "GLFW/glfw3.h"
#include "game/settings.h"
#include <unordered_map>
#include <string>

std::unordered_map<int, std::string> keymap{
		{KEY_SPACE,"Space"},
		{KEY_ESCAPE,"Esc"},
		{KEY_ENTER,"Enter"},
		{KEY_TAB,"Tab"},
		{KEY_BACKSPACE,"Backspace"},
		{KEY_INSERT,"Insert"},
		{KEY_DELETE,"Del"},
		{KEY_RIGHT,"Right"},
		{KEY_LEFT,"Left"},
		{KEY_DOWN,"Down"},
		{KEY_UP,"Up"},
		{KEY_PAGE_UP,"PgUp"},
		{KEY_PAGE_DOWN,"PgDown"},
		{KEY_HOME,"Home"},
		{KEY_END,"End"},
		{KEY_CAPS_LOCK,"CapsLk"},
		{KEY_SCROLL_LOCK,"ScrLk"},
		{KEY_NUM_LOCK,"NumLk"},
		{KEY_PRINT_SCREEN,"PrtSc"},
		{KEY_PAUSE,"Pause"},
		{KEY_F1,"F1"},
		{KEY_F2,"F2"},
		{KEY_F3,"F3"},
		{KEY_F4,"F4"},
		{KEY_F5,"F5"},
		{KEY_F6,"F6"},
		{KEY_F7,"F7"},
		{KEY_F8,"F8"},
		{KEY_F9,"F9"},
		{KEY_F10,"F10"},
		{KEY_F11,"F11"},
		{KEY_F12,"F12"},
		{KEY_LEFT_SHIFT,"LShift"},
		{KEY_LEFT_CONTROL,"LCtrl"},
		{KEY_LEFT_ALT,"LShift"},
		{KEY_LEFT_SUPER,"LWin"},
		{KEY_RIGHT_SHIFT,"RShift"},
		{KEY_RIGHT_CONTROL,"RCtrl"},
		{KEY_RIGHT_ALT,"RShift"},
		{KEY_RIGHT_SUPER,"RWin"},
		{KEY_KB_MENU,"Menu"},
		{KEY_KP_0,"Num 0"},
		{KEY_KP_1,"Num 1"},
		{KEY_KP_2,"Num 2"},
		{KEY_KP_3,"Num 3"},
		{KEY_KP_4,"Num 4"},
		{KEY_KP_5,"Num 5"},
		{KEY_KP_6,"Num 6"},
		{KEY_KP_7,"Num 7"},
		{KEY_KP_8,"Num 8"},
		{KEY_KP_9,"Num 9"},
		{KEY_KP_DECIMAL,"Num ."},
		{KEY_KP_DIVIDE,"Num /"},
		{KEY_KP_MULTIPLY,"Num *"},
		{KEY_KP_SUBTRACT,"Num -"},
		{KEY_KP_ADD,"Num +"},
		{KEY_KP_ENTER,"Num Enter"},
		{KEY_KP_EQUAL,"Num ="}
};

static std::string getKeyStr(int keycode)
{
	if (keycode >= 39 && keycode < 96) {
		char chr = static_cast<char>(keycode);
		return std::string(1,chr);
	}
	else {
		auto it = keymap.find(keycode);
		if (it != keymap.end())
		{
			return it->second;
		}
		else
		{
			return keycode == -1 ? "" : "UNK";
		}
	}
}

std::unordered_map<int, std::string> GenericNames{
	{GLFW_GAMEPAD_BUTTON_CROSS, "A/X"},
	{GLFW_GAMEPAD_BUTTON_CIRCLE, "B/Circle"},
	{GLFW_GAMEPAD_BUTTON_SQUARE, "X/Square"},
	{GLFW_GAMEPAD_BUTTON_TRIANGLE, "Y/Triangle"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "LB"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "RB"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Select"},
	{GLFW_GAMEPAD_BUTTON_START, "Start"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Home"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "LS"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "RS"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"LT"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"RT"}
};

std::unordered_map<int, std::string> XBOXNames{
	{GLFW_GAMEPAD_BUTTON_A, "A"},
	{GLFW_GAMEPAD_BUTTON_B, "B"},
	{GLFW_GAMEPAD_BUTTON_X, "X"},
	{GLFW_GAMEPAD_BUTTON_Y, "Y"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "LB"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "RB"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Back"},
	{GLFW_GAMEPAD_BUTTON_START, "Start"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Guide"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "LS"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "RS"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"LT"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"RT"}
};

std::unordered_map<int, std::string> PS1PS2Names{
	{GLFW_GAMEPAD_BUTTON_CROSS, "X"},
	{GLFW_GAMEPAD_BUTTON_CIRCLE, "Circle"},
	{GLFW_GAMEPAD_BUTTON_SQUARE, "Square"},
	{GLFW_GAMEPAD_BUTTON_TRIANGLE, "Triangle"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "L1"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "R1"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Select"},
	{GLFW_GAMEPAD_BUTTON_START, "Start"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Analog"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "L3"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "R3"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"L2"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"R2"}
};

std::unordered_map<int, std::string> PS3Names{
	{GLFW_GAMEPAD_BUTTON_CROSS, "X"},
	{GLFW_GAMEPAD_BUTTON_CIRCLE, "Circle"},
	{GLFW_GAMEPAD_BUTTON_SQUARE, "Square"},
	{GLFW_GAMEPAD_BUTTON_TRIANGLE, "Triangle"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "L1"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "R1"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Select"},
	{GLFW_GAMEPAD_BUTTON_START, "Start"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Home"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "L3"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "R3"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"L2"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"R2"}
};

std::unordered_map<int, std::string> PS4Names{
	{GLFW_GAMEPAD_BUTTON_CROSS, "X"},
	{GLFW_GAMEPAD_BUTTON_CIRCLE, "Circle"},
	{GLFW_GAMEPAD_BUTTON_SQUARE, "Square"},
	{GLFW_GAMEPAD_BUTTON_TRIANGLE, "Triangle"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "L1"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "R1"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Share"},
	{GLFW_GAMEPAD_BUTTON_START, "Options"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Home"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "L3"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "R3"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"L2"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"R2"}
};

std::unordered_map<int, std::string> PS5Names{
	{GLFW_GAMEPAD_BUTTON_CROSS, "X"},
	{GLFW_GAMEPAD_BUTTON_CIRCLE, "Circle"},
	{GLFW_GAMEPAD_BUTTON_SQUARE, "Square"},
	{GLFW_GAMEPAD_BUTTON_TRIANGLE, "Triangle"},
	{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "L1"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "R1"},
	{GLFW_GAMEPAD_BUTTON_BACK, "Create"},
	{GLFW_GAMEPAD_BUTTON_START, "Options"},
	{GLFW_GAMEPAD_BUTTON_GUIDE, "Home"},
	{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "L3"},
	{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "R3"},
	{GLFW_GAMEPAD_BUTTON_DPAD_UP, "Up"},
	{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Right"},
	{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Down"},
	{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Left"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_X),"LS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_Y),"LS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_X),"RS X"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_Y),"RS Y"},
	{-(1 + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER),"L2"},
	{-(1 + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER),"R2"}
};
std::vector<std::vector<std::string>> controllerTypeNames{
	{},
	{"Controller (XBOX 360 For Windows)",
	"Controller (Xbox 360 Wireless Receiver for Windows)",
	"Controller (Xbox One For Windows) - Wired",
	"Controller (Xbox One For Windows) - Wireless",
	"Xbox Adaptive Controller",
	"Xbox Series Controller"},
	{"PS Controller","PS1 Controller","PS2 Controller"},
	{"PS3 Controller"},
	{"PS4 Controller"},
	{"PS5 Controller"}
};

int getControllerType(const std::string& searchString) {
	for (size_t i = 0; i < controllerTypeNames.size(); ++i) {
		for (size_t j = 0; j < controllerTypeNames[i].size(); ++j) {
			if (controllerTypeNames[i][j] == searchString) {
				return i;
			}
		}
	}
	return 0; //Generic type if not found
}
static std::string getControllerStr(int jid, int input, int type, int direction)
{
	std::unordered_map<int, std::string> &map = GenericNames;
	if (type == 1) map = XBOXNames; //xbox
	else if (type == 2) map = PS1PS2Names; //PS1/2
	else if (type == 3) map = PS3Names; //PS3
	else if (type == 4) map = PS4Names; //PS4
	else if (type == 5) map = PS5Names; //PS5

	auto it = map.find(input);
	if (it != map.end())
	{
		if (input < 0) {
			if (input > -5) {
				return it->second + (direction==-1?"-":"+");
			}
			else return it->second;
		}
		else {
			return it->second;
		}
	}
	else
	{
		return "UNK";
	}
}
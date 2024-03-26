#pragma once
#include "raylib.h"
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
	auto it = keymap.find(keycode);
	if (it != keymap.end())
	{
		return it->second;
	}
	else
	{
		return "UNKNOWN";
	}
}
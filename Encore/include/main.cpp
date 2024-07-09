//
// Created by marie on 08/07/2024.
//

#define SOL_ALL_SAFETIES_ON 1
#define SOL_USE_LUA_HPP 1
#include <sol/sol.hpp>


int main(int argc, char** argv) {
	sol::state lua;

	lua.open_libraries(sol::lib::base);
	lua.script("print('peeseburger')");
	return 0;
}


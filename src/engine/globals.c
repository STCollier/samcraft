#include "globals.h"

struct Globals globals;

void globals_init() {
    const char* CONFIG_FILENAME = "./client/config.lua";
    
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, CONFIG_FILENAME)) {
        ERROR_MSG("Error loading Lua block file:", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    } else {
        LOG_MSG("Loaded file", CONFIG_FILENAME);
    }

    lua_getGlobal(L, "config", "The field 'config' is not a table in", CONFIG_FILENAME);
    globals.debug = lua_getBool(L, "debug", "The field 'debug' is invalid in", CONFIG_FILENAME);
    globals.FOV = lua_getInt(L, "fov", "The field 'fov' is invalid in", CONFIG_FILENAME);

    globals.reach = lua_getFloat(L, "reach", "The field 'reach' is invalid in", CONFIG_FILENAME);
    globals.mouseSensitivity = lua_getFloat(L, "mouse_sensitivity", "The field 'mouse_sensitivity' is invalid in", CONFIG_FILENAME);

    lua_getField(L, "player_speed", "The field 'player_speed' is not a table at", CONFIG_FILENAME);
    globals.playerSpeed.slow = lua_getInt(L, "slow", "The field 'player_speed.slow' is invalid in", CONFIG_FILENAME);
    globals.playerSpeed.normal = lua_getInt(L, "normal", "The field 'player_speed.normal' is invalid in", CONFIG_FILENAME);
    globals.playerSpeed.sprint = lua_getInt(L, "sprint", "The field 'player_speed.sprint' is invalid in", CONFIG_FILENAME);
}
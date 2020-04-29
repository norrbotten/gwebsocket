
#include <sol/sol.hpp>

DLLEXPORT int gmod13_open(lua_State*) {
    Lunar::Loader::Initialize();

    return 0;
}

DLLEXPORT int gmod13_close(lua_State*) {
    Lunar::Loader::Deinitialize();

    return 0;
}
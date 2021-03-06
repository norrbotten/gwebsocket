
workspace "gwebsocket"
    location "build"

    architecture "x32"
    language "C++"
    cppdialect "C++17"

    targetdir "build/bin"
    objdir "build/obj"

    configurations { "debug", "release" }

    filter { "configurations:debug" }
        symbols "On"
        buildoptions { "-std=c++2a" }

    filter { "configurations:release" }
        optimize "Full"
        buildoptions { "-std=c++2a",
                       "-Wall", "-Wextra", "-Wpedantic",
                       "-Wnull-dereference", "-Wmisleading-indentation" }

    filter { }

function includeLunarSOL2()
    includedirs { "ext/lunar-sol2", "ext/lunar-sol2/sol2/include" }
end

function includeIXWebSockets()
    links { "ixwebsocket:static", "z:static", "ssl:static", "crypto:static", "pthread", "dl" }
    includedirs { "ext/ixwebsocket" }
    libdirs { "ext/zlib", "ext/ixwebsocket/build", "ext/openssl" }
end

project "module"
    kind "SharedLib"
    files "src/module/**"

    targetname "gwebsocket"
    targetprefix "gmsv_"
    targetsuffix "_linux"
    targetextension ".dll"

    targetdir "build/module/bin"
    objdir "build/module/obj"

    includeLunarSOL2()
    includeIXWebSockets()

project "test"
    kind "ConsoleApp"
    files "src/test/**"

    targetname "test"
    targetprefix ""
    targetsuffix ""
    targetextension ""

    targetdir "build/test/bin"
    objdir "build/test/obj"

    includeLunarSOL2()
    includeIXWebSockets()


newaction {
    trigger     = "clean",
    description = "clean the software",
    execute     = function()
        os.rmdir("./build")
        os.rmdir("./makefiles")
    end
}

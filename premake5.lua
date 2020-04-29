
workspace "gwebsocket"
    location "makefiles"

    architecture "x32"
    language "C++"
    cppdialect "C++17"

    targetdir "build/bin"
    objdir "build/obj"

    configurations { "debug", "release" }

    filter { "configurations:debug" }
        symbols "On"
        buildoptions { "-Wall", "-Wextra",
                       "-Wnull-dereference", "-Wmisleading-indentation" }

    filter { "configurations:release" }
        optimize "On"
        buildoptions { "-O3", "-Wall", "-Wextra",
                       "-Wnull-dereference", "-Wmisleading-indentation" }

    filter { }

function includeLunarSOL2()
    includedirs { "dependencies/lunar-sol2", "dependencies/lunar-sol2/sol2/include" }
end

function includeIXWebSockets()
    links { "ixwebsocket", "z", "pthread" }
    libdirs { "/usr/local/lib" }
end

project "module"
    kind "SharedLib"
    files "projects/module/**"

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

    targetname "test"
    targetprefix ""
    targetsuffix ""
    targetextension ""

    targetdir "build/test/bin"
    objdir "build/test/obj"

    includeLunarSOL2()
    includeIXWebSockets()

    files "projects/test/**"

newaction {
    trigger     = "clean",
    description = "clean the software",
    execute     = function()
        os.rmdir("./build")
        os.rmdir("./makefiles")
    end
}

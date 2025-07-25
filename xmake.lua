-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsse-ng")

-- set project
set_project("commonlibsse-ng-template")
set_version("0.0.0")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")
add_defines("UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN", "NOMINMAX")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- add requires
add_requires("directxmath", "directxtk")

-- targets
target("skyrim12")
    -- add dependencies to target
    add_packages("directxmath", "directxtk")
    add_syslinks("dxgi", "d3d11", "d3d12")
    if is_mode("debug") then
        add_linkdirs("include/detours/Debug")
    else
        add_linkdirs("include/detours/Release")
    end
    add_links("detours")
    add_deps("commonlibsse-ng")

    -- add commonlibsse-ng plugin
    add_rules("commonlibsse-ng.plugin", {
        name = "skyrim12",
        author = "ProfJack/五脚猫",
        description = "skyrim in 12"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src", "include")
    set_pcxxheader("src/PCH.h")

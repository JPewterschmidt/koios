add_rules(
    "mode.tsan", "mode.ubsan", "mode.asan", 
    "mode.debug", "mode.release", "mode.valgrind"
)

add_requires(
    "gflags", 
    "gtest", 
    "concurrentqueue master",
    "benchmark", 
    "magic_enum"
)

includes("toolpex")


set_languages("c++23", "c17")
set_policy("build.warning", true)
set_policy("build.optimization.lto", false)
set_toolset("cc", "mold", {force = true}) 

if not is_mode("release") then
    add_defines("KOIOS_DEBUG", {force = true})
end

if is_mode("asan") then
    add_cxxflags("-fno-inline", {force = true})
    set_optimize("none", {force = true})
end

if is_mode("release") then
    set_optimize("fastest", {force = true})
end

target("koios")
    set_kind("shared")
    add_deps("toolpex")
    add_packages(
        "gflags", 
        "concurrentqueue", 
        "spdlog", 
        "magic_enum", 
        { public = true }
    )
    set_warnings("all", "error")
    add_cxflags("-Wconversion", { force = true })
    add_syslinks(
        "uring"
    )
    add_files( "src/*.cc")
    add_includedirs(
        "include", 
        { public = true }
    )

target("test")
    set_kind("binary")
    add_packages("concurrentqueue")
    add_cxflags("-Wconversion", { force = true })
    add_deps("koios", "toolpex")
    add_files( "test/*.cc")
    set_warnings("all", "error")
    add_packages(
        "gtest", "spdlog"
    )
    after_build(function (target)
        os.exec(target:targetfile())
        print("xmake: unittest complete.")
    end)
    on_run(function (target)
        --nothing
    end)
    
target("example")
    set_kind("binary")
    add_packages("")
    add_cxflags("-Wconversion", { force = true })
    add_deps("koios", "toolpex")
    add_files( "example/*.cc")
    add_syslinks("spdlog")
    set_policy("build.warning", true)
    add_packages(
        "gflags", 
        "concurrentqueue"
    )
    

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--


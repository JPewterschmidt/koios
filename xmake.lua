add_rules(
    "mode.tsan", "mode.ubsan", "mode.asan", 
    "mode.debug", "mode.release", "mode.valgrind"
)

add_requires(
    "gflags", 
    "gtest", 
    "concurrentqueue master",
    "benchmark"
)

includes("toolpex")

add_includedirs(
    "include", 
    "toolpex/include", 
    { public = true }
)

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
        "spdlog"
    )
    set_warnings("all", "error")
    add_cxflags("-Wconversion", { force = true })
    add_syslinks(
        "uring"
    )
    add_files( "src/*.cc")

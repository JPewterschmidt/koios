add_rules(
    "mode.tsan", "mode.ubsan", "mode.asan", 
    "mode.debug", "mode.release", "mode.valgrind"
)

add_requires(
    "gflags", 
    "gtest", 
    "concurrentqueue master",
    "benchmark", 
    "magic_enum",
    "fmt"
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
        "fmt",
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
        os.execv(target:targetfile(), {"--gtest_color=yes"})
        print("xmake: unittest complete.")
    end)
    on_run(function (target)
        --nothing
    end)
    
target("example")
    set_kind("binary")
    add_cxflags("-Wconversion", { force = true })
    add_deps("koios", "toolpex")
    add_files( "example/*.cc")
    add_packages("spdlog")
    set_policy("build.warning", true)
    add_packages(
        "gflags", 
        "concurrentqueue"
    )
    

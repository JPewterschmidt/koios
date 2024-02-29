add_rules(
    "mode.tsan", "mode.ubsan", "mode.asan", 
    "mode.debug", "mode.release"
)

add_requires(
    "fmt", 
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

set_languages("c++2b", "c17")
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
        "fmt", 
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

--target("test")
--    set_kind("binary")
--    add_packages("concurrentqueue")
--    add_cxflags("-Wconversion", { force = true })
--    add_deps("koios", "toolpex")
--    add_files( "test/*.cc")
--    set_warnings("all", "error")
--    add_packages(
--        "gtest", "fmt", "spdlog"
--    )
--    after_build(function (target)
--        os.exec(target:targetfile())
--        print("xmake: unittest complete.")
--    end)
--    on_run(function (target)
--        --nothing
--    end)
--    
--target("example")
--    set_kind("binary")
--    add_packages("")
--    add_cxflags("-Wconversion", { force = true })
--    add_deps("koios", "toolpex")
--    add_files( "example/*.cc")
--    add_syslinks("spdlog")
--    set_policy("build.warning", true)
--    add_packages(
--        "fmt", "gflags", 
--        "concurrentqueue"
--    )

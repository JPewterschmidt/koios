add_rules(
    "mode.tsan", "mode.ubsan", "mode.asan", 
    "mode.debug", "mode.release"
)

add_requires(
    "fmt", 
    "gflags", 
    "gtest", 
    "concurrentqueue master",
    "benchmark", 
    "botan"
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

target("koios")
    set_kind("shared")
    add_deps("toolpex")
    add_packages(
        "fmt", 
        "gflags", 
        "concurrentqueue", 
        "botan"
    )
    set_warnings("all", "error")
    add_cxflags("-Wconversion", { force = true })
    add_syslinks(
        "spdlog", 
        "uring"
    )
    add_files( "src/*.cc")

--target("koios-test")
--    set_kind("binary")
--    add_packages("concurrentqueue")
--    add_cxflags("-Wconversion", { force = true })
--    add_deps("koios", "toolpex")
--    set_warnings("all", "error")
--    add_files( "test/*.cc")
--    add_packages(
--        "gtest", "fmt", "spdlog",
--        "botan"
--    )
--    after_build(function (target)
--        os.exec(target:targetfile())
--        print("xmake: unittest complete.")
--    end)
--    on_run(function (target)
--        --nothing
--    end)
    
--target("koios-example")
--    set_kind("binary")
--    add_packages("")
--    add_cxflags("-Wconversion", { force = true })
--    add_deps("koios", "toolpex")
--    add_files( "example/*.cc")
--    add_syslinks("spdlog")
--    set_policy("build.warning", true)
--    add_packages(
--        "fmt", "gflags", 
--        "concurrentqueue", 
--        "botan"
--    )
    


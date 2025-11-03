set_project("aax-bruteforce")
set_languages("c++20")

-- Custom PGO rule
rule("pgo")
    on_config(function (target)
        target:set("pgo_profile_data", "default.profdata")
        target:set("pgo_test_args", "23456789abcde23456789abcde23456789abcdef")  
        target:set("pgo_timeout", "60")
    end)
    
    on_build(function (target)
        import("core.project.depend")
        
        local targetfile = target:targetfile()
        local profgen = targetfile .. "_profgen"
        local profile_data = target:get("pgo_profile_data")
        local test_args = target:get("pgo_test_args")
        local timeout = target:get("pgo_timeout")
        
        -- Get build flags
        local cxxflags = target:get("cxxflags") or {}
        local ldflags = target:get("ldflags") or {}
        local sources = target:sourcefiles()
        
        depend.on_changed(function ()
            print("Building PGO instrumented binary...")
            
            -- Profile generation build
            local profgen_flags = table.join(cxxflags, {"-fprofile-instr-generate"})
            local profgen_ldflags = table.join(ldflags, {"-fprofile-instr-generate"})
            
            os.vrunv("clang++", table.join(profgen_flags, profgen_ldflags, {"-o", profgen}, sources))
            
            -- Run profiling
            print("Running profile collection...")
            os.vrunv("timeout", {"--signal=SIGINT", timeout, profgen, test_args}, {try = true})
            
            -- Merge profile data  
            print("Merging profile data...")
            os.vrunv("llvm-profdata", {"merge", "-sparse", "default.profraw", "-o", profile_data})
            
            -- Final optimized build
            print("Building PGO optimized binary...")
            local pgo_flags = table.join(cxxflags, {"-fprofile-instr-use=" .. profile_data})
            local pgo_ldflags = table.join(ldflags, {"-fprofile-instr-use=" .. profile_data})
            
            os.vrunv("clang++", table.join(pgo_flags, pgo_ldflags, {"-o", targetfile}, sources))
            
            -- Cleanup
            os.rm(profgen)
            
        end, {files = sources})
    end)
    
    on_clean(function (target)
        local profile_data = target:get("pgo_profile_data")
        os.rm(profile_data)
        os.rm("default.profraw")
        os.rm("*.gcda") 
        os.rm("*.gcno")
    end)
rule_end()

-- Common settings
add_cxxflags("-O3", "-std=c++20", "-pthread", "-Wall", "-Wextra", "-march=native", "-mtune=native", "-flto")
add_syslinks("pthread")

-- Regular build
target("main")
    set_kind("binary")
    add_files("*.cpp")

-- Static build
target("main_static")
    set_kind("binary") 
    add_files("*.cpp")
    add_ldflags("-static")

-- PGO build with custom rule
target("main_pgo")
    set_default(true)
    set_kind("binary")
    add_files("*.cpp")
    add_rules("pgo")

-- Static PGO build
target("main_static_pgo")
    set_kind("binary")
    add_files("*.cpp") 
    add_ldflags("-static")
    add_rules("pgo")
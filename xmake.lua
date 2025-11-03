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
        import("core.tool.compiler")
        
        local targetfile = target:targetfile()
        local profgen = targetfile .. "_profgen"
        local profile_data = target:get("pgo_profile_data")
        local test_args = target:get("pgo_test_args")
        local timeout = target:get("pgo_timeout")
        
        -- Create target directory
        os.mkdir(path.directory(targetfile))
        
        depend.on_changed(function ()
            print("Building PGO instrumented binary...")
            
            -- Get compiler instance
            local compinst = compiler.load("cxx", {target = target})
            
            -- Profile generation build
            local profgen_flags = table.join(target:get("cxxflags") or {}, {"-fprofile-instr-generate"})
            local profgen_ldflags = table.join(target:get("ldflags") or {}, {"-fprofile-instr-generate"})
            
            compinst:link(target:sourcefiles(), profgen, {
                linkflags = profgen_ldflags,
                cxxflags = profgen_flags
            })
            
            -- Run profiling
            print("Running profile collection...")
            os.vrunv("timeout", {"--signal=SIGINT", timeout, profgen, test_args}, {try = true})
            
            -- Merge profile data  
            print("Merging profile data...")
            os.vrunv("llvm-profdata", {"merge", "-sparse", "default.profraw", "-o", profile_data})
            
            -- Final optimized build
            print("Building PGO optimized binary...")
            local pgo_flags = table.join(target:get("cxxflags") or {}, {"-fprofile-instr-use=" .. profile_data})
            local pgo_ldflags = table.join(target:get("ldflags") or {}, {"-fprofile-instr-use=" .. profile_data})
            
            compinst:link(target:sourcefiles(), targetfile, {
                linkflags = pgo_ldflags,
                cxxflags = pgo_flags  
            })
            
            -- Cleanup
            os.rm(profgen)
            
        end, {files = target:sourcefiles()})
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
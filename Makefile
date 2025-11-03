# AAX Bruteforce Makefile
# ======================

# Compiler and flags
CXX = clang++
CXXFLAGS = -O3 -std=c++20 -pthread -Wall -Wextra -flto -march=native -mtune=native
LDFLAGS =
LDLIBS =

# PGO (Profile Guided Optimization) settings
PROFILE_GEN_FLAGS = -fprofile-instr-generate
PROFILE_USE_FLAGS = -fprofile-instr-use=$(PROFILE_DATA)
PROFILE_DATA = default.profdata
PGO_TEST_ARGS = 23456789abcde23456789abcde23456789abcdef
PGO_TIMEOUT = 60

# Source files
SOURCES = main.cpp cpu.cpp helpers.cpp

# Static linking warning
STATIC_WARNING = @echo "Note: Static linking requires static versions of system libraries"; \
                 @echo "Consider installing: glibc-static, libpthread-static, libm-static"

# ======================
# Default target
# ======================

all: main_pgo

# ======================
# Standard builds
# ======================

main: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LDLIBS)

main_static: $(SOURCES)
	$(STATIC_WARNING)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -static -o $@ $(SOURCES) $(LDLIBS)

# ======================
# PGO builds
# ======================

main_pgo: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -o $@_profgen $(SOURCES) $(LDLIBS)
	-timeout --signal=SIGINT $(PGO_TIMEOUT) ./$@_profgen $(PGO_TEST_ARGS)
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LDLIBS)
	rm -f $@_profgen

main_static_pgo: $(SOURCES)
	$(STATIC_WARNING)
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -static -o $@_profgen $(SOURCES) $(LDLIBS)
	-timeout --signal=SIGINT $(PGO_TIMEOUT) ./$@_profgen $(PGO_TEST_ARGS)
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -static -o $@ $(SOURCES) $(LDLIBS)
	rm -f $@_profgen

# ======================
# Convenience targets
# ======================

static: main_static_pgo

# ======================
# Cleanup
# ======================

clean:
	rm -f main main_static main_pgo main_static_pgo 
	rm -f main_pgo_profgen main_static_pgo_profgen 
	rm -f *.gcda *.gcno *.profraw $(PROFILE_DATA)

# ======================
# Phony targets
# ======================

.PHONY: all static clean
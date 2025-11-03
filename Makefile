CXX = clang++
CXXFLAGS = -O3 -std=c++20 -pthread -Wall -Wextra -flto -march=native -mtune=native
LDFLAGS =
LDLIBS =
PROFILE_GEN_FLAGS = -fprofile-instr-generate
PROFILE_USE_FLAGS = -fprofile-instr-use=$(PROFILE_DATA)

PROFILE_DATA = default.profdata
SOURCES = main.cpp cpu.cpp helpers.cpp

all: main_pgo

.PHONY: static
static: main_static_pgo

main: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LDLIBS)

main_static: $(SOURCES)
	@echo "Note: Static linking requires static versions of system libraries"
	@echo "Consider installing: glibc-static, libpthread-static, libm-static"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -static -o $@ $(SOURCES) $(LDLIBS)

main_pgo: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -o $@_profgen $(SOURCES) $(LDLIBS)
	-timeout --signal=SIGINT 60 ./$@_profgen 23456789abcde23456789abcde23456789abcdef
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LDLIBS)
	rm -f $@_profgen

main_static_pgo: $(SOURCES)
	@echo "Note: Static linking requires static versions of system libraries"
	@echo "Consider installing: glibc-static, libpthread-static, libm-static"
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -static -o $@_profgen $(SOURCES) $(LDLIBS)
	-timeout --signal=SIGINT 60 ./$@_profgen 23456789abcde23456789abcde23456789abcdef
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -static -o $@ $(SOURCES) $(LDLIBS)
	rm -f $@_profgen

clean:
	rm -f main main_static main_pgo main_static_pgo main_pgo_profgen main_static_pgo_profgen *.gcda *.gcno *.profraw $(PROFILE_DATA)

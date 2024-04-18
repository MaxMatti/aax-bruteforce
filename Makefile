CXX = clang++
CXXFLAGS = -O3 -std=c++20 -pthread -Wall -Wextra -flto -march=native -mtune=native
LDFLAGS =
LDLIBS =
PROFILE_GEN_FLAGS = -fprofile-instr-generate
PROFILE_USE_FLAGS = -fprofile-instr-use=$(PROFILE_DATA)

PROFILE_DATA = default.profdata

all: main_pgo main_static_pgo

main_pgo: main.cpp cpu.cpp helpers.cpp
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	-timeout --signal=SIGINT 600 ./$@ 23456789abcde23456789abcde23456789abcdef
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main_static_pgo: main.cpp cpu.cpp helpers.cpp
	$(CXX) $(CXXFLAGS) $(PROFILE_GEN_FLAGS) $(LDFLAGS) -static -o $@ $^ $(LDLIBS)
	-timeout --signal=SIGINT 600 ./$@ 23456789abcde23456789abcde23456789abcdef
	llvm-profdata merge -sparse default.profraw -o $(PROFILE_DATA)
	$(CXX) $(CXXFLAGS) $(PROFILE_USE_FLAGS) $(LDFLAGS) -static -o $@ $^ $(LDLIBS)

clean:
	rm -f main_pgo main_static_pgo *.gcda *.gcno *.profraw $(PROFILE_DATA)

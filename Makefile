main:
	clang++ -O3 -std=c++20 -pthreads -Wall -Wextra -flto -march=native -mtune=native -o main main.cpp cpu.cpp helpers.cpp
clean:
	rm main

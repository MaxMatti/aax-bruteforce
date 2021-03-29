# aax-bruteforce

Bruteforces keys for audibles aax-files.

Exhausts the keyspace on my Ryzen 3700X in 4 minutes and 11 seconds which equals 3865 CPU-seconds as it's using all 16 threads.

## Installation

On Linux: grab a binary from the [releases-page](https://github.com/MaxMatti/aax-bruteforce/releases) or build it yourself, then run it.

## Building

Dependencies: Clang (I used version 11.1.0)
```bash
git clone --depth=1 https://github.com/MaxMatti/aax-bruteforce.git
make
```

## Usage

Just run the executable, if you want you can do it with some parameters:
```
./main 23456789abcde23456789abcde23456789abcdef 16
```
where the long string is your hash which you can find out so:
```
ffprobe audiobook.aax 2>&1 | grep checksum
```
and the number is the amount of threads and can be left out for optimal usage of your CPU.

#include <array>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cpu.h"
#include "helpers.h"

// for the profiling step of profile guided optimization
void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  // Cleanup and close up stuff here
  // Terminate program
  exit(signum);
}

template <class T, size_t count = 64>
struct RingBuffer {
  RingBuffer() = default;

  void push(const T& elem) {
    while (!try_push(elem)) {
      std::this_thread::yield();
    }
  }

  void pull(T& elem) {
    while (!try_pull(elem)) {
      std::this_thread::yield();
    }
  }

  bool try_push(const T& elem) {
    size_t head = _head.load(std::memory_order_relaxed);
    size_t tail = _tail.load(std::memory_order_acquire);
    if (head >= tail + count) {
      return false;  // buffer full
    }
    _buffer[head % count] = elem;
    _head.store(head + 1, std::memory_order_release);
    return true;
  }

  bool try_pull(T& elem) {
    size_t head = _head.load(std::memory_order_acquire);
    size_t tail = _tail.load(std::memory_order_relaxed);
    if (head <= tail) {
      return false;  // buffer empty
    }
    elem = _buffer[tail % count];
    _tail.store(tail + 1, std::memory_order_release);
    return true;
  }

 private:
  std::array<T, count> _buffer;
  std::atomic<size_t> _head = 0;
  std::atomic<size_t> _tail = 0;
};

void worker_thread(RingBuffer<std::array<char, 20>, 64>& thread_communication,
                   std::uint64_t thread_id, std::uint64_t thread_count) {
  const char* constant =
      "\x77\x21\x4d\x4b\x19\x6a\x87\xcd\x52\x00\x45\xfd\x20\xa5\x1d\x67";
  size_t counter = 0;
  char* bytes = reinterpret_cast<char*>(&counter);
  alignas(64) char stage0[64] = {0};
  alignas(64) char stage1[64] = {0};
  alignas(64) char stage2[64] = {0};
  alignas(64) std::uint32_t block[80] = {0};
  std::array<char, 20> result;
  for (counter = thread_id; counter < (1ul << 32); counter += thread_count) {
    memcpy(stage0, bytes, 16);
    memcpy(stage0 + 16, bytes, 4);
    cpu::sha1(stage0, 20, stage1 + 16, block);
    memcpy(stage1, constant, 16);
    memcpy(stage1 + 36, bytes, 4);
    cpu::sha1(stage1, 40, stage2 + 16, block);
    memcpy(stage2, stage1 + 16, 16);
    cpu::sha1(stage2, 32, result.data(), block);
    thread_communication.push(result);
  }
}

int main(int argc, char const* argv[]) {
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  std::string filename;
  unsigned int thread_count;

  if (argc > 1) {
    filename = argv[1];
  } else {
    filename = "hashlist.bin";
  }
  if (argc > 2) {
    thread_count = std::stoul(std::string{argv[2]});
  } else {
    thread_count = std::thread::hardware_concurrency();
  }

  auto result_stream = std::ofstream{filename};
  std::uint64_t keyspace = (1ul << 32) / thread_count;
  std::vector<std::thread> threads;
  std::vector<RingBuffer<std::array<char, 20>, 64>> thread_communication;
  for (std::size_t i = 0; i < thread_count; ++i) {
    threads.emplace_back(worker_thread,
                         std::ref(thread_communication.emplace_back()), i,
                         thread_count);
  }
  for (std::size_t i = 0; i < (1ul << 32); ++i) {
    std::array<char, 20> buffer;
    thread_communication[i % thread_count].pull(buffer);
    result_stream.write(buffer.data(), 20);
  }
  for (auto& t : threads) {
    t.join();
  }

  return 0;
}

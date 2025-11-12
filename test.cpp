#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "cpu.h"
#include "helpers.h"

// Copy of the worker_thread function from main.cpp for testing
void worker_thread_test(char* hash, std::uint64_t counter,
                        std::uint64_t counter_end, bool* found,
                        std::uint32_t* result_counter) {
  const char* constant =
      "\x77\x21\x4d\x4b\x19\x6a\x87\xcd\x52\x00\x45\xfd\x20\xa5\x1d\x67";
  char* hash_local = static_cast<char*>(calloc(20, sizeof(char)));
  char* bytes = reinterpret_cast<char*>(&counter);
  char* stage0 = static_cast<char*>(calloc(64, sizeof(char)));
  char* stage1 = static_cast<char*>(calloc(64, sizeof(char)));
  char* stage2 = static_cast<char*>(calloc(64, sizeof(char)));
  char* stage3 = static_cast<char*>(calloc(20, sizeof(char)));
  std::uint32_t* block =
      static_cast<std::uint32_t*>(calloc(80, sizeof(std::uint32_t)));
  memcpy(hash_local, hash, 20);
  for (; counter < counter_end; ++counter) {
    memcpy(stage0, constant, 16);
    memcpy(stage0 + 16, bytes, 4);
    cpu::sha1(stage0, 20, stage1 + 16, block);
    memcpy(stage1, constant, 16);
    memcpy(stage1 + 36, bytes, 4);
    cpu::sha1(stage1, 40, stage2 + 16, block);
    memcpy(stage2, stage1 + 16, 16);
    cpu::sha1(stage2, 32, stage3, block);
    if (memcmp(stage3, hash, 20) == 0) {
      std::cout << "Found match! Activation bytes: "
                << helpers::base16(bytes, 4) << std::endl;
      *found = true;
      *result_counter = counter;
      break;
    }
  }
  free(hash_local);
  free(stage0);
  free(stage1);
  free(stage2);
  free(stage3);
  free(block);
}

// Helper function to convert hex string to bytes
void hex_to_bytes(const std::string& hex, char* bytes) {
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byte_string = hex.substr(i, 2);
    bytes[i / 2] = static_cast<char>(std::stoul(byte_string, nullptr, 16));
  }
}

// Test function with specific known input-output pairs
bool test_specific_cases() {
  std::cout << "Testing specific input-output pairs..." << std::endl;

  struct TestCase {
    std::string input_hex;
    std::string expected_hash;
    std::string description;
  };

  TestCase test_cases[] = {
      {"6d42f806", "f57bb42022923b41c8650646c2cfd3adb896cbb5", "Test case 1"},
      {"3af9791d", "b97c50d09cc16dbf9fb7751db7564f59f7f2660c", "Test case 2"},
      {"5401b005", "8180a416f45392af6013240019b70940bf925364", "Test case 3"}};

  bool all_passed = true;

  for (size_t i = 0; i < 3; ++i) {
    std::cout << test_cases[i].description << ": " << test_cases[i].input_hex
              << " => " << test_cases[i].expected_hash << std::endl;

    // Convert input hex to counter value
    std::uint32_t counter;
    hex_to_bytes(test_cases[i].input_hex, reinterpret_cast<char*>(&counter));

    // Convert expected hash to bytes
    char expected_hash[20];
    hex_to_bytes(test_cases[i].expected_hash, expected_hash);

    // Test the worker thread function with this specific counter
    bool found = false;
    std::uint32_t result_counter = 0;
    worker_thread_test(expected_hash, counter, counter + 1, &found,
                       &result_counter);

    if (found && result_counter == counter) {
      std::cout << "  ✓ PASSED: Found expected result" << std::endl;
    } else {
      std::cout << "  ✗ FAILED: Expected to find counter " << counter
                << " but found: "
                << (found ? std::to_string(result_counter) : "nothing")
                << std::endl;
      all_passed = false;
    }
  }

  return all_passed;
}

int main() {
  bool success = test_specific_cases();

  if (success) {
    std::cout << std::endl << "All tests completed successfully!" << std::endl;
    return 0;
  } else {
    std::cout << std::endl << "Some tests failed!" << std::endl;
    return 1;
  }
}
#include "cpu.h"
#include "helpers.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

// converts base16-string to byte-string
std::string base16(std::string input) {
	// 8 bytes in a "normal" string, 4 bytes in a base16-encoded (hex) string
	std::string result(input.size() * 2, 0);
	char characters[] = "0123456789abcdef";
	for (unsigned int i = 0; i < input.size(); ++i) {
		result[i * 2] = characters[(input[i] >> 4) & 15];
		result[i * 2 + 1] = characters[input[i] & 15];
	}
	return result;
}

void thread(char* hash, std::uint64_t counter, std::uint64_t counter_end) {
	const char* constant = "\x77\x21\x4d\x4b\x19\x6a\x87\xcd\x52\x00\x45\xfd\x20\xa5\x1d\x67";
	char* hash_local = static_cast<char*>(calloc(20, sizeof(char)));
	char* bytes = reinterpret_cast<char*>(&counter);
	char* stage0 = static_cast<char*>(calloc(64, sizeof(char)));
	char* stage1 = static_cast<char*>(calloc(64, sizeof(char)));
	char* stage2 = static_cast<char*>(calloc(64, sizeof(char)));
	char* stage3 = static_cast<char*>(calloc(20, sizeof(char)));
	std::uint32_t* block = static_cast<std::uint32_t*>(calloc(80, sizeof(std::uint32_t)));
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
			std::cout << "Activation bytes: " << helpers::base16(bytes, 4) << std::endl;
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

int main(int argc, char const *argv[]) {
	std::string hash_base16;
	unsigned int threadcount;

	if (argc > 1) {
		hash_base16 = argv[1];
	} else {
		std::cout << "Enter hash: ";
		std::cin >> hash_base16;
	}
	if (argc > 2) {
		threadcount = std::stoul(std::string{argv[2]});
	} else {
		threadcount = std::thread::hardware_concurrency();
	}

	if (hash_base16.length() != 40) {
		std::cout << "Wrong hash length.\n";
		exit(-1);
	}

	char* hash = static_cast<char*>(calloc(20, sizeof(char)));
	for (int i = 0; i < 20; ++i) {
		hash[i] = ((hash_base16[i * 2] + (hash_base16[i * 2] & 0x40 ? 9 : 0)) << 4) | ((hash_base16[i * 2 + 1] + (hash_base16[i * 2 + 1] & 0x40 ? 9 : 0)) & 0xf);
	}

	std::uint64_t keyspace = (1ul << 32) / threadcount;
	std::vector<std::thread> threads;
	for (unsigned int i = 0; i < threadcount; ++i) {
		threads.emplace_back(thread, hash, keyspace * i, keyspace * (i + 1));
	}
	for (auto& t : threads) {
		t.join();
	}

	return 0;
}

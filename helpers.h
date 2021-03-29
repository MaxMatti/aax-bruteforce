#pragma once

#include <string>

namespace helpers {

	// calculates the ceiling of a division in a fast way.
	template <typename T> T fastCeil(T denominator, T divisor);
	unsigned int fastCeil(unsigned int denominator, unsigned int divisor);

	// returns a string with length random characters
	std::string getRandomStr(unsigned int length);

	// fills input with a random string of length length, requires allocated space of length + 1
	void getRandomStr(unsigned int length, unsigned char* input);

	// converts byte-string to base16-string
	std::string base16(std::string input);
	std::string base16(const char* input);
	std::string base16(const char* input, unsigned int length);
	std::string base16(const unsigned char* input);
	std::string base16(const unsigned char* input, unsigned int length);

	// converts byte-string to base32-string
	std::string base32(std::string input, char padding);
	std::string base32(std::string input);
	std::string base32(const char* input);
	std::string base32(const unsigned char* input);
	std::string base32(const unsigned char* input, unsigned int length);

	// converts base32-string to byte-string
	std::string base32toStr(std::string input);
}

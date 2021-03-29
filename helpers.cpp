#include <algorithm>

#include "helpers.h"

namespace helpers {

	// calculates the ceiling of a division in a fast way.
	template <typename T> T fastCeil(T denominator, T divisor) {
		return (denominator + divisor - 1) / divisor;
	}

	// calculates the ceiling of a division in a fast way.
	unsigned int fastCeil(unsigned int denominator, unsigned int divisor) {
		return (denominator + divisor - 1) / divisor;
	}

	// returns a string with length random characters
	std::string getRandomStr(unsigned int length) {
		static auto randchar = []() -> char {
			return ' ' + rand() % 94;
		};
		std::string result(length, 0);
		std::generate_n(result.begin(), length, randchar);
		return result;
	}

	// fills input with a random string of length length, requires allocated space of length + 1
	void getRandomStr(unsigned int length, unsigned char* input) {
		static auto randchar = []() -> char {
			return ' ' + rand() % 94;
		};
		for (unsigned int i = 0; i < length; ++i) {
			input[i] = randchar();
		}
		input[length] = 0;
	}

	// converts byte-string to base16-string
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

	// converts byte-string to base16-string
	std::string base16(const char* input) {
		std::string input_str(input);
		return base16(input_str);
	}

	// converts byte-string to base16-string
	std::string base16(const char* input, unsigned int length) {
		std::string input_str(input, length);
		return base16(input_str);
	}

	// converts byte-string to base16-string
	std::string base16(const unsigned char* input) {
		std::string input_str(reinterpret_cast<const char*>(input));
		return base16(input_str);
	}

	// converts byte-string to base16-string
	std::string base16(const unsigned char* input, unsigned int length) {
		std::string input_str(reinterpret_cast<const char*>(input), length);
		return base16(input_str);
	}

	// converts byte-string to base32-string
	std::string base32(std::string input, char padding) {
		unsigned int final_output_size = fastCeil((unsigned int) input.size() * 8, (unsigned int) 5);
		input.append(5 - input.size() % 5, 0);
		std::string result(input.size() * 8 / 5, 0); // 8 bytes in a "normal" string, 5 bytes in a base32-encoded string
		char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ23456789";
		unsigned int output_pos = 0;
		for (unsigned int i = 0; i < input.size(); i += 5, output_pos += 8) {
			result[output_pos    ] = characters[( input[i    ] >> 3) & 31];                               // 31 base10 = 11111 base2
			result[output_pos + 1] = characters[((input[i    ] << 2) & 28) | ((input[i + 1] >> 6) & 3)];  // 28 base10 = 11100 base2, 3 base10 = 11 base2
			result[output_pos + 2] = characters[( input[i + 1] >> 1) & 31];                               // 31 base10 = 11111 base2
			result[output_pos + 3] = characters[((input[i + 1] << 4) & 16) | ((input[i + 2] >> 4) & 15)]; // 16 base10 = 10000 base2, 15 base10 = 01111 base2
			result[output_pos + 4] = characters[((input[i + 2] << 1) & 30) | ((input[i + 3] >> 7) & 1)];  // 30 base10 = 11110 base2, 1 base10 = 00001 base2
			result[output_pos + 5] = characters[( input[i + 3] >> 2) & 31];                               // 31 base10 = 11111 base2
			result[output_pos + 6] = characters[((input[i + 3] << 3) & 24) | ((input[i + 4] >> 5) & 7)];  // 24 base10 = 11000 base2, 7 base10 = 00111 base2
			result[output_pos + 7] = characters[  input[i + 4]       & 31];                               // 31 base10 = 11111 base2
		}
		result.resize(final_output_size);
		result.append(8 - result.size() % 8, padding);
		return result;
	}

	// converts byte-string to base32-string
	std::string base32(std::string input) {
		return base32(input, '=');
	}

	// converts byte-string to base32-string
	std::string base32(const char* input) {
		std::string input_str(input);
		return base32(input_str, '=');
	}

	// converts byte-string to base32-string
	std::string base32(const unsigned char* input) {
		std::string input_str(reinterpret_cast<const char*>(input));
		return base32(input_str, '=');
	}

	// converts byte-string to base32-string
	std::string base32(const unsigned char* input, unsigned int length) {
		std::string input_str(reinterpret_cast<const char*>(input), length);
		return base32(input_str, '=');
	}

	// converts base32-string to byte-string
	std::string base32toStr(std::string input) {
		unsigned int final_output_size = fastCeil((unsigned int) input.size() * 5, (unsigned int) 8);
		input.append(8 - input.size() % 8, 0);
		std::string result(input.size() / 8 * 5, 0);

		// convert 2-7 so that every characters first 3 bits are 0 and the last 5 bits are a correct binary representation.
		for (unsigned int i = 0; i < input.size(); ++i) {
			if (input[i] < 64) {
				input[i] += 41;
			}
			input[i] &= 31;
		}

		unsigned int output_pos = 0;
		for (unsigned int i = 0; i < input.size(); i += 8, output_pos += 5) {
			result[output_pos    ] = input[i    ] << 3 | input[i + 1] >> 2;
			result[output_pos + 1] = input[i + 1] << 6 | input[i + 2] << 1 | input[i + 3] >> 4;
			result[output_pos + 2] = input[i + 3] << 4 | input[i + 4] >> 1;
			result[output_pos + 3] = input[i + 4] << 7 | input[i + 5] << 2 | input[i + 6] >> 3;
			result[output_pos + 4] = input[i + 6] << 5 | input[i + 7];
		}
		result.resize(final_output_size);
		return result;
	}
}


#include <cstdint>
#include <climits>

#include "cpu.h"

namespace cpu {
	// swaps the endian of any datatype. Source: http://stackoverflow.com/a/4956493
	template <typename T> T swap_endian(T u) {
		static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
		union {
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;
		source.u = u;
		for (unsigned int k = 0; k < sizeof(T); k++) {
			dest.u8[k] = source.u8[sizeof(T) - k - 1];
		}
		return dest.u;
	}

	// implementation of the s function as described in section 3. of spec
	std::uint32_t sha1_helper_s(std::uint32_t input, unsigned char offset) {
		return (input << offset) | (input >> (32 - offset));
	}

	// implementation of the f function as described in section 5. of spec
	std::uint32_t sha1_helper_f(unsigned char nr, const std::uint32_t& b, const std::uint32_t& c, const std::uint32_t& d) {
		if (nr < 20) {
			return (b & c) | ((b ^ 0xFFFFFFFF) & d);
		} else if (nr < 40) {
			return b ^ c ^ d;
		} else if (nr < 60) {
			return (b & c) | (b & d) | (c & d);
		} else if (nr < 80) {
			return b ^ c ^ d;
		} else {
			return 0;
		}
	}

	// representation of the K variables as described in section 5. of spec
	std::uint32_t sha1_helper_K(unsigned char nr) {
		if (nr < 20) {
			return 0x5A827999;
		} else if (nr < 40) {
			return 0x6ED9EBA1;
		} else if (nr < 60) {
			return 0x8F1BBCDC;
		} else if (nr < 80) {
			return 0xCA62C1D6;
		} else {
			return 0;
		}
	}

	// calculates the sha1 sum
	void sha1(void* input_buffer, const unsigned int& input_buffer_size, void* output, std::uint32_t* current_block) {
		// in case the machine uses big endian we need to swap some bytes later:
		bool convert_endians;
		{
			std::uint32_t endian = 0x08040201;
			if (*reinterpret_cast<unsigned char*>(&endian) == 1) {
				convert_endians = 1;
			} else if (*reinterpret_cast<unsigned char*>(&endian) == 8) {
				convert_endians = 0;
			} else {
				return;
			}
		}
		// copying input_buffer to own storage area with larger size:
		const unsigned int input_size = (input_buffer_size + 72) & 0xFFFFFFC0;
		// 73 because 512bit blocks (64bytes) and ending in length (64bit aka 8 bytes) and 1 byte because of padding starting with 0b10000000
		// 73 = 64 + 8 + 1
		// but for some reason when using 73 some results differ from OpenSSLs implementation, fixed by using 72. TODO: investigate this.

		// then applying floor function
		unsigned char* input = static_cast<unsigned char*>(input_buffer);
		memset(input + input_buffer_size + 1, 0, (input_size - input_buffer_size - 5) * sizeof(char));

		// 4. filling up input buffer according to spec
		*(input + input_buffer_size) = 0x80; // set first bit to 1, others to 0
		{
			std::uint64_t tmp;
			if (convert_endians) { // convert endianness in case of big endian
				tmp = swap_endian<std::uint64_t>(input_buffer_size << 3);
			} else {
				tmp = input_buffer_size << 3;
			}
			memcpy(input + input_size - 8, &tmp, 8);
			// These are to check wether the input string was corrupted:
			// std::string a(reinterpret_cast<char*>(input), input_size);
			// std::cerr << base16(a) << std::endl;
		}

		// 6.1 actual hash algorithm:

		// initializing result buffer (h0-h4):
		std::uint32_t* result = reinterpret_cast<std::uint32_t*>(output);
		result[0] = 0x67452301;
		result[1] = 0xefcdab89;
		result[2] = 0x98badcfe;
		result[3] = 0x10325476;
		result[4] = 0xc3d2e1f0;

		bool free_current_block = !current_block;
		// initializing block buffer, tmp "word" and "words" A-E as described in 6.2
		if (!current_block) {
			current_block = static_cast<std::uint32_t*>(calloc(80, sizeof(std::uint32_t)));
		} else {
			memset(current_block, 0, 80 * sizeof(std::uint32_t));
		}
		if (!current_block) {
			return;
		} // return nullptr if calloc failed.
		std::uint32_t tmp[] = {0, 0, 0, 0, 0, 0}; // tmp and then a-e

		// processing block by block
		for (unsigned int i = 0; i < input_size; i += 64) {

			// copy current block to buffer
			memcpy(current_block, input + i, 64);

			// convert endianness in case of big endian
			for (unsigned int j = 0; j < 64 && convert_endians; ++j) {
				current_block[j] = swap_endian<std::uint32_t>(current_block[j]);
			}

			// 6.2 (b) calculate the rest of the current block
			for (unsigned int j = 16; j < 80; ++j) {
				current_block[j] = sha1_helper_s(current_block[j - 3] ^ current_block[j - 8] ^ current_block[j - 14] ^ current_block[j - 16], 1);
			}

			// 6.2 (c) fill a-e
			memcpy(tmp + 1, result, 5 * sizeof(int32_t));

			// 6.2 (d) wobble around
			for (unsigned char j = 0; j < 80; ++j) {
				tmp[0] = sha1_helper_s(tmp[1], 5) + sha1_helper_f(j, tmp[2], tmp[3], tmp[4]) + tmp[5] + current_block[j] + sha1_helper_K(j);
				tmp[5] = tmp[4];
				tmp[4] = tmp[3];
				tmp[3] = sha1_helper_s(tmp[2], 30);
				tmp[2] = tmp[1];
				tmp[1] = tmp[0];
			}

			// 6.2 (e) wobble around a little bit more
			result[0] += tmp[1];
			result[1] += tmp[2];
			result[2] += tmp[3];
			result[3] += tmp[4];
			result[4] += tmp[5];
		}
		// convert endianness in case of big endian
		for (unsigned int j = 0; j < 5 && convert_endians; ++j) {
			result[j] = swap_endian<std::uint32_t>(result[j]);
		}
		// free memory
		if (free_current_block) {
			free(current_block);
		}
	}
}

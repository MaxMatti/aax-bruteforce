#pragma once

#include <algorithm>
#include <cstring>

namespace cpu {
	// swaps the endian of any datatype. Source: http://stackoverflow.com/a/4956493
	template <typename T> T swap_endian(T u);

	// implementation of the s function as described in section 3. of spec
	std::uint32_t sha1_helper_s(std::uint32_t input, unsigned char offset);

	// implementation of the f function as described in section 5. of spec
	std::uint32_t sha1_helper_f(unsigned char nr, const std::uint32_t& b, const std::uint32_t& c, const std::uint32_t& d);

	// representation of the K variables as described in section 5. of spec
	std::uint32_t sha1_helper_K(unsigned char nr);

	// calculates the sha1 sum
	void sha1(void* input_buffer, const unsigned int& input_buffer_size, void* output, std::uint32_t* current_block);
}

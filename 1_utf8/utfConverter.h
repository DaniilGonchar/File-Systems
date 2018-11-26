#include <iostream>
#include <vector>
#include <exception>
#include <algorithm>

using std::vector;
using std::cout;
using std::cin;
using std::endl;

enum State { first_byte, two_bytes, three_bytes, four_bytes };

uint8_t number_of_octets(const uint32_t &number)
{
	if (number < static_cast<uint32_t >(1 << 7)) return 1;
	if (number < static_cast<uint32_t >(1 << 11)) return 2;
	if (number < static_cast<uint32_t >(1 << 16)) return 3;
	if (number < static_cast<uint32_t >(1 << 21)) return 4;
	throw std::runtime_error(__PRETTY_FUNCTION__);
}

uint8_t number_of_symbol_bytes(const uint8_t &byte)
{
	if (byte >> 7 == 0) return 1;
	if (byte >> 5 == 0b110) return 2;
	if (byte >> 4 == 0b1110) return 3;
	if (byte >> 3 == 0b11110) return 4;
	throw std::runtime_error(__PRETTY_FUNCTION__);
}

bool consistency_check(const vector<uint8_t> &x)
{
	State state = first_byte;
	uint8_t bit_counter = 0;
	for (const auto &item:x)
	{
		const uint8_t cur_bit = item;
		switch (state)
		{
			case first_byte:
			{
				if (cur_bit >> 7 == 0) continue;
				else if (cur_bit >> 5 == 0b110)
				{
					bit_counter++;
					state = two_bytes;
					continue;
				}
				else if (cur_bit >> 4 == 0b1110)
				{
					bit_counter++;
					state = three_bytes;
					continue;
				}
				else if (cur_bit >> 3 == 0b11110)
				{
					bit_counter++;
					state = four_bytes;
					continue;
				}
				
				return false;
			}
			case two_bytes:
			{
				if (cur_bit >> 6 == 0b10)
				{
					state = first_byte;
					bit_counter = 0;
					continue;
				}
				else return false;
			}
			case three_bytes:
			{
				if (cur_bit >> 6 == 0b10 && bit_counter < 2)
				{
					bit_counter++;
					continue;
				}
				else if (cur_bit >> 6 == 0b10 && bit_counter == 2)
				{
					state = first_byte;
					bit_counter = 0;
					continue;
				}
				else return false;
			}
			case four_bytes:
			{
				if (cur_bit >> 6 == 0b10 && bit_counter < 3)
				{
					bit_counter++;
					continue;
				}
				else if (cur_bit >> 6 == 0b10 && bit_counter == 3)
				{
					state = first_byte;
					bit_counter = 0;
					continue;
				}
				else return false;
			}
		}
	}
	
	return state == first_byte;
}

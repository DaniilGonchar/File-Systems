#include "utfConverter.h"

// compile with c++17
vector<uint8_t> to_utf8(const vector<uint32_t> &x)
{
	auto shift_shrink = [](const uint32_t &number, uint8_t shift)
	{
		return 0b10000000 | (0b00111111 & static_cast<uint8_t >(number >> shift));
	};
	
	vector<uint8_t> result;
	for (const auto number: x)
	{
		uint8_t bytes_num = number_of_octets(number);
		switch (bytes_num)
		{
			case 1:
			{
				result.push_back((uint8_t) number);
				continue;
			}
			case 2:
			{
				result.push_back(0b11000000 | (number >> 6));
				result.push_back(shift_shrink(number, 0));
				continue;
			}
			case 3:
			{
				result.push_back(0b11100000 | (number >> 12));
				result.push_back(shift_shrink(number, 6));
				result.push_back(shift_shrink(number, 0));
				continue;
			}
			default:
			{
				result.push_back(0b11110000 | (number >> 18));
				result.push_back(shift_shrink(number, 12));
				result.push_back(shift_shrink(number, 6));
				result.push_back(shift_shrink(number, 0));
				continue;
			}
		}
	}
	
	return result;
}

vector<uint32_t> from_utf8(const vector<uint8_t> &x)
{
	if (!consistency_check(x))
	{
		throw std::runtime_error(__PRETTY_FUNCTION__);
	}
	
	auto shrink_shift = [](const uint8_t &x, uint8_t shift)
	{
		return static_cast<uint32_t >(0b00111111 & x) << shift;
	};
	
	vector<uint32_t> result;
	for (auto number = x.cbegin(); number != x.cend();)
	{
		uint8_t bytes_number = number_of_symbol_bytes(*number);
		switch (bytes_number)
		{
			case 1:
			{
				result.push_back(*number);
				++number;
				continue;
			}
			case 2:
			{
				result.push_back((((uint32_t) 0b00011111 & (*number)) << 6) + shrink_shift(*++number, 0));
				++number;
				continue;
			}
			case 3:
			{
				result.push_back((((uint32_t) 0b00001111 & (*number)) << 12) + shrink_shift(*++number, 6) + shrink_shift(*++number, 0));
				++number;
				continue;
			}
			case 4:
			{
				result.push_back((((uint32_t) 0b00000111 & (*number)) << 18) + shrink_shift(*++number, 12) + shrink_shift(*++number, 6) + shrink_shift(*++number, 0));
				++number;
				continue;
			}
			default:
				continue;
		}
	}
	
	return result;
}

int main(void)
{
	vector<uint8_t> test_from_utf = {0b11100000, 0b10100100, 0b10111001, 0b11100010, 0b10000010, 0b10001111, 0b11100010, 0b10000010, 0b10111111, 0b11100010, 0b10000011, 0b10010100, 0b11100010, 0b10000011, 0b10100100};
	vector<uint32_t> test_to_utf = {0x0939, 0x208F, 0x20BF, 0x20D4, 0x20E4};
	vector<uint8_t> result_8 = to_utf8(test_to_utf);
	vector<uint32_t> result_32 = from_utf8(test_from_utf);

	cout << "Character\t \u0939 \t converting to UTF code" << test_to_utf[0] << endl;
	cout << "Character\t \u208F \t converting to UTF code" << test_to_utf[1] << endl;
	cout << "Character\t \u20BF \t converting to UTF code" << test_to_utf[2] << endl;
	cout << "Character\t \u20D4 \t converting to UTF code" << test_to_utf[3] << endl;
	cout << "Character\t \u20E4 \t converting to UTF code" << test_to_utf[4] << endl;
	
	bool to_utf8_passed = std::equal(test_from_utf.begin(), test_from_utf.end(), result_8.begin(), result_8.end());
	bool from_utf8_passed = std::equal(test_to_utf.begin(), test_to_utf.end(), result_32.begin(), result_32.end());
	std::string to_utf8_result = to_utf8_passed ? "to_utf8 PASSED" : "to_utf8 ERROR";
	std::string from_utf8_result = from_utf8_passed ? "from_utf8 PASSED" : "to_utf8 ERROR";
	
	cout << to_utf8_result << endl;
	cout << from_utf8_result << endl;
}

#include <print>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <bit>

bool is_little_endian() {
	int testNumber = 1;
	char lowest_byte = testNumber && 0x000000FF;

	if (lowest_byte == 1) return true;
	return false;
}

int swap_endianess(int n) {
	int lowest_one = n & 0x000000FF;
	int lowest_two = n & 0x0000FF00;
	int lowest_third = n & 0x00FF0000;
	int lowest_fourth = n & 0xFF000000;

	int new_n = lowest_one << 24 | lowest_two << 8 | lowest_third >> 8 | lowest_fourth >> 24;

	std::print(std::cout, "Original: {:08X}\nSwapped: {:08X}", n, new_n);
	return new_n;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Error: program accepts two paameters\nInput and output file.");
		return 1;
	}

	//open input and output file
	std::ifstream input(argv[1]/*, std::ios::binary*/);
	if (input.fail()) {
		std::println(std::cerr, "Error: impossible open {} input file", argv[1]);
		return 1;
	}

	std::ofstream output(argv[2], std::ios::binary);
	if (output.fail()) {
		std::println(std::cerr, "Error: impossible open {} output file", argv[2]);
		return 1;
	}

	// read numbers from input file .txt
	std::vector <int>v;
	int num;
	while (input >> num) {
		std::print(std::cout, "Hex: {:08X}\tDec: {}\n", num, num);
		v.push_back(num);
	}

	for (int num : v) {
		output.write((char*)&num, sizeof(num));
	}
	return 0;
}
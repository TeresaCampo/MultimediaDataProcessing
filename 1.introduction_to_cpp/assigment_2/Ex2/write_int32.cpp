#include <print>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <bit>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Error: program accepts two paameters\nInput and output file.");
		return 1;
	}

	std::ifstream input(argv[1]/*, std::ios::binary*/);
	if (input.fail()) {
		std::println(std::cerr, "Error: impossible open {} input file", argv[1]);
		return 1;
	}

	std::ofstream output(argv[1], std::ios::binary);
	if (output.fail()) {
		std::println(std::cerr, "Error: impossible open {} output file", argv[2]);
		return 1;
	}

	std::vector <int>v;
	int num;
	while (input >> num) {
		v.push_back(num);
	}

	//operation to execute if big endian representation
	if (std::endian::native == std::endian::big) {
		std::cout << "big-endian\n";
		for (int element : v) {
			int l_endinan = std::uint32_t(element);
		}
	}
	if constexpr (std::endian::native == std::endian::little)
		std::cout << "little-endian\n";
	return 0;
}
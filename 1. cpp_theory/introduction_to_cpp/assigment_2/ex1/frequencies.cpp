#include <cstdio>
#include <print>
#include <iostream>
#include <fstream>
#include <set>
#include <iterator>
#include <vector>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Error: program accepts two paameters\nInput and output file.");
		return 1;		
	}

	std::ifstream input(argv[1], std::ios::binary);
	if (input.fail()) {
		std::println(std::cerr, "Error: impossible open {} input file", argv[1]);
		return 1;
	}

	//open output file
	std::ofstream output(argv[2]);
	if (output.fail()) {
		std::println(std::cerr, "Error: impossible open {} input file", argv[2]);
		return 1;
	}

	// read input bytes in a multiset
	std::istream_iterator<char> start(input);
	std::istream_iterator<char> stop;
	std::multiset<unsigned char> file_bytes(start, stop);

	std::set<unsigned char> file_bytes_set(file_bytes.begin(), file_bytes.end());
	for (const char& element: file_bytes_set )
	{
		int frequency = file_bytes.count(element);
		std::println(std::cout, "{:d}", element);
		std::print(output, "{:02X}\t{}\n", element, frequency);
	}
	return 0;
}
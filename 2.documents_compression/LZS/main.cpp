#include "lzs.h"
#include <iostream>
#include <fstream>
int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "ERROR: program accepts 2 parameters.\n";
		return EXIT_FAILURE;
	}
	
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cerr << "ERROR: fail opening file " << argv[1] << ".\n";
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::cerr << "ERROR: fail opening file " << argv[2] << ".\n";
		return EXIT_FAILURE;
	}

	lzs_decompress(is, os);
}
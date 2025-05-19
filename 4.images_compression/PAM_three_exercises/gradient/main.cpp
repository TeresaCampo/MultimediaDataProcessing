#include <iostream>
#include <fstream>
#include <cstdio>

#include <array>

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Error: one parameter is needed.\n";
		return EXIT_FAILURE;
	}

	std::ofstream os(argv[1], std::ios::binary);
	if (!os) {
		std::cerr << "Error: fail opening output file.\n";
		return EXIT_FAILURE;
	}

	os << "P7\n";
	os << "WIDTH 256\n";
	os << "HEIGHT 256\n";
	os << "DEPTH 1\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";

	std::array<char, 256> v;
	for (int i = 0; i < 256; i++) {
		v.fill(static_cast<char>(i));
		os.write(v.data(), v.size());
	}
}
#include <cstdio>
#include <print>
#include <fstream>

#include <string>
#include <vector>
#include <map>
struct bitreader {
	std::ifstream& is_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};

	uint32_t read_one_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}

		uint32_t val = (buffer_ >> (8 - n_)) & 1;
		n_--;
		return val;
	}
	uint32_t read_n_bits(uint8_t nbits) {
		uint32_t val = 0;
		for (uint8_t pos = 0; pos < nbits; pos++){
			val |= read_one_bit() << pos;
		}
		return val;
	}
};


bool read_RIFF_header(std::ifstream& is) {
	std::string header(4, ' ');
	is.read(header.data(), 4);
	if (header != "RIFF") {
		std::println("Error: worng RIFF header.");
		return false;
	}

	uint32_t dummy = 0;
	is.read(reinterpret_cast<char*>(&dummy), 4);

	is.read(header.data(), 4);
	if (header != "WEBP") {
		std::println("Error: worng RIFF header.");
		return false;
	}
	is.read(header.data(), 4);
	if (header != "VP8L") {
		std::println("Error: worng RIFF header.");
		return false;
	}

	is.read(reinterpret_cast<char*>(&dummy), 4);
	dummy = 0;
	is.read(reinterpret_cast<char*>(&dummy), 1);
	if (dummy != 47) {
		std::println("Error: worng RIFF header.");
		return false;
	}

	return true;
}
bool read_bitstream(std::ifstream& is) {
	bitreader br(is);
	int image_width = br.read_n_bits(14) + 1;
	int image_height = br.read_n_bits(14) + 1;
	int alpha = br.read_n_bits(1);
	int version = br.read_n_bits(3);

	int transform = br.read_n_bits(1);
	int color_cache = br.read_n_bits(1);
	int meta_prefix = br.read_n_bits(1);

	std::map<uint8_t, uint8_t> code_lengths;
	//read prefix code specifications
	//#1
	if (br.read_one_bit() == 0) { //normal code length
		int num_code_lengths = 4 + br.read_n_bits(4);
		int kCodeLengthCodes = 19;
		std::vector<uint8_t> kCodeLenghtCodeOrder = { 17, 18, 0, 1, 2,3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::map <uint8_t, uint8_t>code_length_code_lengths;

		for (int i = 0; i < num_code_lengths; i++) {
			code_length_code_lengths[kCodeLenghtCodeOrder[i]] = br.read_n_bits(3);
		}
	}
	else {  //simple cod elength
		int num_symbols = br.read_n_bits(1) + 1;
		int is_first_8bits = br.read_n_bits(1);
		uint8_t s = br.read_n_bits(1 + 7 * is_first_8bits);
		code_lengths[s] = 1;

		if (num_symbols == 2) {
			s = br.read_n_bits(8);
			code_lengths[s] = 1;
		}
	}
	return true;
}
int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 arguments, not {}", argc - 1);
		return EXIT_SUCCESS;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}", argv[1]);
		return	EXIT_FAILURE;
	}

	if (!read_RIFF_header(is)) return EXIT_FAILURE;
	read_bitstream(is);
}
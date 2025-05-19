#include <print>
#include <cstdio>
#include <fstream>
#include <vector>
#include<iterator>
#include <map>
#include <cmath>
#include <bit>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val,
	size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val,
	size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&val), size);
}


class bitreader {
	std::istream& is_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;

	uint32_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is) {}

	// Read n bits into u from MSB to LSB
	std::istream& read(uint32_t& u, size_t n) {
		//u = 0;
		while (n-- > 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}
};

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;

	void writebit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}

	// Write the n least significant bits of u from MSB to LSB
	std::ostream& write(uint32_t u, size_t n) {
		while (n-- > 0) {
			writebit(u >> n);
		}
		return os_;
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			writebit(bit);
		}
		return os_;
	}
};

uint32_t map(int32_t x) {
	if (x < 0) {
		return static_cast<uint32_t>(x * -2);
	}
	else {
		return static_cast<uint32_t>(x * 2 +1);
	}

}

int compress(const std::string& inputFile, const std::string& outputFile) {
	// open input file
	std::ifstream inFile(inputFile/*, std::ios::binary*/);
	if (!inFile) {
		std::println(stderr, "Error: impossible to open {} file", inputFile);
		return (EXIT_FAILURE);
	}

	// read numbers from input file
	std::istream_iterator<int> start(inFile);
	std::istream_iterator<int> end;
	std::vector<int> v(start, end);

	// open output file(binary mode)
	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile) {
		std::println(stderr, "Error: impossible to open {} file", outputFile);
		return EXIT_FAILURE;
	}

	bitwriter br(outFile);
	// write each number in output file
	
	//alternative to not copute the log and do not use floating point
	for (const auto& el : v) {
		//std::bit_width(el);
		br.write(0, std::floor(std::log2(map(el))));
		br.write(map(el), std::floor(std::log2(map(el)) + 1));
	}
	return EXIT_SUCCESS;
}

int32_t invmap(uint32_t x) {
	if (x % 2 == 0) {
		return static_cast<int32_t>(x) / -2;
	}
	else {
		return static_cast<int32_t>(x - 1) / 2;
	}
}

int decode(const std::string& inputFile, const std::string& outputFile) {
	// open input file
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile) {
		std::println(stderr, "Error: impossible to open {} file", inputFile);
		return (EXIT_FAILURE);
	}

	// open output file
	std::ofstream outFile(outputFile/*, std::ios::binary*/);
	if (!outFile) {
		std::println(stderr, "Error: impossible to open {} file", outputFile);
		return (EXIT_FAILURE);
	}

	bitreader br(inFile);
	uint32_t val = 0;
	uint32_t zero_counter = 0;

	while (1) {
		br.read(val, 1);
		while (val == 0) {
			++zero_counter;
			br.read(val, 1);
		}
		if (inFile.eof()) {
			break;
		}

		br.read(val, zero_counter);
		outFile << invmap(val) << '\n';
		zero_counter = 0;
		val = 0;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::println(stderr, "Error: program accepts 3 parameters");
		return EXIT_FAILURE;
	}

	// read mode
	if (std::string(argv[1]) == "c") {
		compress(argv[2], argv[3]);
	}
	else if (std::string(argv[1]) == "d") {
		decode(argv[2], argv[3]);
	}
	else {
		std::println(stderr, "Error: first parameter is {}, but should be 'c' or 'd'", argv[1]);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
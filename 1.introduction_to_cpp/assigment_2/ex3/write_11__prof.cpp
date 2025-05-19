#include <cstdlib>
#include <bitset>
#include <print>
#include <iostream>
#include <fstream>
#include <vector>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val,size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;		//number of bits inside bitwriter
public:
	bitwriter (std::ostream& os) : os_(os){
	}

	void writebit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);		//equal to buffer_ * 2 + bit
		++n_;

		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
	}
public:
	~bitwriter() {
		flush();
	}
public:
	//write the n least significant bits of u from MSB to LSB
	std::ostream& write(uint32_t u, size_t n) {
		for (size_t i = n; i -- > 0; ) {
			writebit(u >> i);
		}
		return os_;
	}
public:
	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0){
			writebit(bit);
		}
		return os_;
	}
};

int main(int argc, char** argv) {
	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream inputFile(argv[1]/*, std::ios::binary*/);
	if (!inputFile) {
		return EXIT_FAILURE;
	}
	vector<int> v{
		istream_iterator<int>(inputFile),
		istream_iterator<int>()
	};

	ofstream outputFile(argv[2], std::ios::binary);
	if (!outputFile) {
		return EXIT_FAILURE;
	}

	bitwriter bw(outputFile);
	for (const auto& x : v) {
		bw.write(x, 11);
	}

	return EXIT_SUCCESS;
}

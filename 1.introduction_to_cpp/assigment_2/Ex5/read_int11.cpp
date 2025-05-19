#include <cstdlib>
#include <bitset>
#include <print>
#include <iostream>
#include <fstream>
#include <vector>

template<typename T>
std::istream& raw_read(std::istream& is, T& val,
	size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	std::istream& is_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;		//number of bits inside bitwriter

public:
	bitreader(std::istream& is) : is_(is) {
	}

	uint32_t readbit() {
		if (n_ == 0) {
			raw_read(is_,buffer_);
			n_ = 8;
		}

		--n_;
		return (buffer_ >> n_) & 1; //equal to return buffer_/ (1<<n_)
		//????????????????prendi l'ultimo enon il primo?
	}

	//read n bits into u from MSB to LSB
	std::istream& read(uint32_t& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}
};

int main(int argc, char** argv) {
	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream inputFile(argv[1], std::ios::binary);
	if (!inputFile) {
		return EXIT_FAILURE;
	}
	vector<int> v{
		istream_iterator<int>(inputFile),
		istream_iterator<int>()
	};

	ofstream outputFile(argv[2]/*, std::ios::binary*/);
	if (!outputFile) {
		return EXIT_FAILURE;
	}

	bitreader br(inputFile);
	vector<int> v;
	uint32_t val;
	while (br.read(val, 11)) {
		int32_t realVal = val;
		if (val > 1023) {
			realVal = realVal - 1024 - 1204;
		}
		v.push_back(realVal);
	}

	return EXIT_SUCCESS;
}

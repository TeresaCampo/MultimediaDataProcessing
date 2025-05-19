#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <iterator>
#include <vector>

struct buffermanager {
	std::vector<char> buffer_;
	std::istream& is_;

public:
	buffermanager(std::istream& is): is_(is){}

public:
	std::vector<char> fill_buffer() {
		char val;
		is_.read(&val, 1);

		while (buffer_.size() < 128 and !is_.eof()) {
			buffer_.push_back(val);
			is_.read(&val, 1);
		}
		return buffer_;
	}

public: 
	std::vector<char> shift_buffer(size_t n) {
		// remove first n
		auto init_pos = buffer_.begin();
		auto end_pos = buffer_.begin();
		std::advance(end_pos, n);
		buffer_.erase(init_pos, end_pos);
		
		// refill buffer
		return fill_buffer();
	}
};

int encoder(const std::string& inputFile, const std::string& outputFile) {
	// open both files in binary mode
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: impossible open input file " << inputFile<<'\n';
		return EXIT_FAILURE;
	}

	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile) {
		std::cerr << "Error: impossible open input file " << outputFile << '\n';
		return EXIT_FAILURE;
	}

	// read 128 bytes
	//check first 2, set mode copy or run
	//run -> go head untill first different is found, go back one
	//copy -> go head untill first equal is found, go back 2
	//create message, shift buffer

	buffermanager bm(inFile);
	std::vector<char> buffer= bm.fill_buffer();
	int n = 0;
	int counter = 0;
	char val = 0;

	while (buffer.size() >=2) {
		if (buffer[0] == buffer[1]) {
			//run
			val = buffer[0];
			n = 2;
			for (int i = 2; i < buffer.size(); i++) {
				if (buffer[i] == val) ++n;
				else break;
			}
			counter = 257 - n;
			outFile.write(reinterpret_cast<char *>( & counter), 1);
			outFile.write(&val, 1);
		}
		else {
			//copy
			std::vector<char> copy_value;
			copy_value.push_back(buffer[0]);
			copy_value.push_back(buffer[1]);
			n = 2;
			for (int i = 2; i < buffer.size(); i++) {
				if (buffer[i] != copy_value[i - 1]) {
					copy_value.push_back(buffer[i]);
					n++;
				}
				else {
					n--; //ingore copy_value[i-1]
					break;
				}
			}
			counter = n - 1;
			outFile.write(reinterpret_cast<char*>(&counter), 1);
			for (int c = 0; c < n; c++) {
				outFile.write(&copy_value[c], 1);
			}
		}
		buffer = bm.shift_buffer(n);
		n = 0;
	}
	if (buffer.size() == 1) {
		//last copy
		counter = 0;
		outFile.write(reinterpret_cast<char*>(&counter), 1);
		outFile.write(&buffer[0], 1);
	}
	val = 128;
	outFile.write(&val, 1);

	return EXIT_SUCCESS;
}

int decoder(const std::string& inputFile, const std::string& outputFile) {
	// open both files in binary mode
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: impossible open input file " << inputFile << '\n';
		return EXIT_FAILURE;
	}

	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile) {
		std::cerr << "Error: impossible open input file " << outputFile << '\n';
		return EXIT_FAILURE;
	}

	unsigned char n;
	char r;
	char val;
	inFile.read(reinterpret_cast<char *>(&n), 1);

	while (static_cast<unsigned char>(n)!=128) {

		if (n < 128) {
			n++;
			//copy
			while (n--> 0) {
				inFile.read(&val, 1);
				outFile.write(&val, 1);
			}
			n = 0;
		}
		if (n > 129) {
			//run
			r = 257 - n;
			inFile.read(&val, 1);

			while (r--> 0) {
				outFile.write(&val, 1);
			}
		}
		inFile.read(reinterpret_cast<char*>(&n), 1);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Error: programs accepts 3 arguments," << argc - 1 << " were provided\n";
		return EXIT_FAILURE;
	}

	// encoder
	if (std::string(argv[1]) == "c") {
		return encoder(std::string(argv[2]), std::string(argv[3]));
	}
	//decoder
	if (std::string(argv[1]) == "d") {
		return decoder(std::string(argv[2]), std::string(argv[3]));
	}
	else {
		std::cerr << "Error: first argument should be 'c' or 'd'\n";
		return EXIT_FAILURE;
	}
}
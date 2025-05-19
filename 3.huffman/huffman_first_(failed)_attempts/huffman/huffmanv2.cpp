#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdint>

#include <iterator>
#include <string>
#include <map>
#include <vector>
#include <bitset>

#include <algorithm>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val,
	size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

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
	std::ostream& writeNbits(uint32_t u, size_t n) {
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

struct codesBuilder {
	std::map<char, std::pair<int, uint32_t>> codesTable_;
	//byte, number bit of code, code in binary

	codesBuilder() {}
	~codesBuilder() {}

	void insert_symbol(char symbol) {
		codesTable_[symbol] = { 0,0 };
	}

	void update_symbol_code(char symbol, char bit) {
		std::pair<int, uint32_t > code = codesTable_[symbol];
		code.second = code.second | (bit << code.first);
		code.first += 1;
		codesTable_[symbol] = code;
	}

	std::map<char, std::pair<int, uint32_t>> getCodes() {
		return codesTable_;
	}

	void DEBUG_printCodesTable() {
		for (auto el : codesTable_) {
			std::cout << el.first << "->" << std::bitset<32>(el.second.second) << "\n";
		}
	}
};

void insertMergedAsFirst(std::vector<std::pair<std::vector<char>, float>>& f, std::pair<std::vector<char>, float> merged) {
	auto it = f.begin();
	for (int i = 0; i < f.size(); i++) {
		if (merged.second < f[i].second) {
			f.insert(it + i + 1, merged);
			return;
		}
	}
	f.insert(it, merged);
	return;
}

void insertMergedAsLast(std::vector<std::pair<std::vector<char>, float>>& f, std::pair<std::vector<char>, float> merged) {
	auto it = f.begin();
	for (int i = f.size() - 1; i >= 0; i--) {
		if (merged.second <= f[i].second) {
			f.insert(it + i + 1, merged);
			return;
		}
	}
	f.insert(it, merged);
	return;
}
bool descendingFreq(std::pair<std::vector<char>, float> el1, std::pair<std::vector<char>, float> el2) {
	return el1.second > el2.second;
}
int compressor(std::string input, std::string output) {
	std::ifstream inFile(input, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: fail opening " << input << "\n";
		return EXIT_FAILURE;
	}

	std::ofstream outFile(output, std::ios::binary);
	if (!outFile) {
		std::cerr << "Error: fail opening " << output << "\n";
		return EXIT_FAILURE;
	}

	// count frequencies
	std::map<char, float> f_tmp;
	char val;
	uint32_t counter = 0;
	while (!inFile.get(val).eof()) {
		f_tmp[val] += 1;
		std::cout << val;
		++counter;
	}
	std::cout << "\n\n";

	//create vector to order (symbols-frequencies)
	//create codesBuilder
	std::vector<std::pair<std::vector<char>, float>> f;
	codesBuilder cbuilder;
	for (auto& element : f_tmp) {
		element.second /= counter;

		cbuilder.insert_symbol(element.first);
		//f.push_back(std::pair< std::vector<char>, float>(std::vector<char>(1,element.first),element.second));
		f.push_back({ std::vector<char>(1,element.first), element.second });

	}
	std::sort(f.begin(), f.end(), descendingFreq);

	//start algorithm
	std::pair<std::vector<char>, float> last;
	std::pair<std::vector<char>, float>	semiLast;
	std::pair<std::vector<char>, float>	merge;
	while (f.size() > 1) {
		//select last two elements
		last = f.back();
		f.pop_back();

		semiLast = f.back();
		f.pop_back();

		//update codes
		//attribute 0 to last, 1 to semi-last
		for (auto s : last.first) {
			cbuilder.update_symbol_code(s, 0);
		}
		for (auto s : semiLast.first) {
			cbuilder.update_symbol_code(s, 1);
		}

		//merge last two lines
		merge.first.clear();
		merge.first.insert(merge.first.end(), semiLast.first.begin(), semiLast.first.end());
		merge.first.insert(merge.first.end(), last.first.begin(), last.first.end());

		merge.second = last.second + semiLast.second;

		//set merge as last of elements with its own frequency
		insertMergedAsLast(f, merge);
	}

	//print table in file
	bitwriter bw(outFile);
	std::map<char, std::pair<int, uint32_t>> codesTable = cbuilder.getCodes();

	//print HUFFMAN1
	outFile << "HUFFMAN1";
	//print number of table entries
	if (codesTable.size() == 256) 	bw.writeNbits(0, 8);
	if (codesTable.size() < 256)	bw.writeNbits(codesTable.size(), 8);
	else {
		std::cerr << "Error: more than 256 elements in huffman table\n";
		return EXIT_FAILURE;
	}
	//print table
	for (auto el : codesTable) {
		bw.writeNbits(el.first, 8);
		bw.writeNbits(el.second.first, 5);
		bw.writeNbits(el.second.second, el.second.first);
	}
	//print number of symbols in the original file(BIG ENDIAN)
	bw.writeNbits((counter >> 24), 8);
	bw.writeNbits((counter >> 16), 8);
	bw.writeNbits((counter >> 8), 8);
	bw.writeNbits((counter), 8);
	//print data
	//read input file from beginning again
	inFile.clear();
	inFile.seekg(0);

	std::pair<int, uint32_t> symbol_code;
	while (!inFile.get(val).eof()) {
		symbol_code = codesTable[val];
		bw.writeNbits(symbol_code.second, symbol_code.first);
	}
	cbuilder.DEBUG_printCodesTable();

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Error: program accepts 3 parameters, " << argc - 1 << " were provided\n.";
		return EXIT_FAILURE;
	}

	if (std::string(argv[1]) == "c") {
		return compressor(argv[2], argv[3]);
	}
}
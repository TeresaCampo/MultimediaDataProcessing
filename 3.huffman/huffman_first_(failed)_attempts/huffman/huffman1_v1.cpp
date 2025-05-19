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

class bitWriter {
	uint32_t buffer_ = { 0 };
	size_t n_ = 0;
	std::ofstream& os_;
public:
	bitWriter(std::ofstream& os) : os_(os) {};
public:
	~bitWriter() {
		flush();
	};

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			oneBit(bit);
		}
		return os_;
	}

	void oneBit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;

		if (n_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 0;
			buffer_ = 0;
		}
	}

public:
	void write_n_bits(uint32_t bit, size_t nbit) {
		while (nbit-- > 0) {
			oneBit(bit >> nbit);
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
		++counter;
	}

	//create vector to order (symbols-frequencies)
	//create codesBuilder
	std::vector<std::pair<std::vector<char>, float>> f;
	codesBuilder cbuilder;
	for (auto& element : f_tmp) {
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
	bitWriter bw(outFile);
	std::map<char, std::pair<int, uint32_t>> codesTable = cbuilder.getCodes();

	//print HUFFMAN1
	outFile << "HUFFMAN1";
	//print number of table entries
	if (codesTable.size() == 256) 	bw.write_n_bits(0, 8);
	if (codesTable.size() < 256)	bw.write_n_bits(codesTable.size(), 8);
	else {
		std::cerr << "Error: more than 256 elements in huffman table\n";
		return EXIT_FAILURE;
	}
	//print table
	for (auto el : codesTable) {
		bw.write_n_bits(el.first, 8);
		bw.write_n_bits(el.second.first, 5);
		bw.write_n_bits(el.second.second, el.second.first);
	}
	//print number of symbols in the original file(BIG ENDIAN)
	bw.write_n_bits(counter, 32);

	//read input file from beginning again
	inFile.clear();
	inFile.seekg(0);

	std::pair<int, uint32_t> symbol_code;
	while (!inFile.get(val).eof()) {
		symbol_code = codesTable[val];
		bw.write_n_bits(symbol_code.second, symbol_code.first);
	}

	return EXIT_SUCCESS;
}

//decompressor stuff

class bitReader {
	int n_ = 0;
	uint8_t buffer_ = 0;
	std::ifstream& is_;


public:
	bitReader(std::ifstream& is) : is_(is) {};
	~bitReader() {};

	uint32_t oneBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*> (&buffer_), 1);
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}
	std::ifstream& NBit(uint32_t& val, size_t nbits) {
		val = 0;
		while (nbits-- > 0) {
			val = (val << 1) | oneBit();
		}
		return is_;
	}
};

int decompressor(std::string input, std::string output) {
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

	bitReader br(inFile);
	uint32_t val;

	//alternative
	// 
	// std::string header(8, ' ');

	std::string algorithm("HUFFMAN1");
	for (int i = 0; i < 8; i++) {
		br.NBit(val, 8);
		if (static_cast<char>(val) != algorithm[i]) {
			std::cerr << "Error: file is not encoded with huffman algorithm";
			return EXIT_FAILURE;
		}
	}

	uint32_t nTableEntries;
	br.NBit(nTableEntries, 8);
	if (nTableEntries == 0) nTableEntries = 256;

	std::map<int, std::vector< std::pair<uint32_t, char>>> codesDict;
	uint32_t symbol;
	uint32_t code;
	uint32_t codeLenght;

	while (nTableEntries-- > 0) {
		br.NBit(symbol, 8);
		br.NBit(codeLenght, 5);
		br.NBit(code, codeLenght);

		codesDict[codeLenght].push_back({ code, symbol });
	}

	//read number of symbols
	uint32_t nSymbols = 0;

	br.NBit(val, 8);
	nSymbols += (val & 0xff) << 24;
	br.NBit(val, 8);
	nSymbols += (val & 0xff) << 16;
	br.NBit(val, 8);
	nSymbols += (val & 0xff) << 8;
	br.NBit(val, 8);
	nSymbols += (val & 0xff);

	uint32_t nextBit;
	uint32_t codeRead = 0;
	char n = 0;

	bitWriter bw(outFile);

	//read and decode symbols
	while (!br.NBit(nextBit, 1).eof() and nSymbols > 0) {
		++n;
		codeRead = codeRead << 1 | (nextBit & 1);

		auto it = codesDict.find(n);
		if (it != codesDict.end()) {
			for (auto el : it->second) {
				if (el.first == codeRead) {
					bw.write_n_bits(el.second, 8);

					nSymbols--;
					codeRead = 0;
					n = 0;
					break;
				}
			}
		}
	}
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
	if (std::string(argv[1]) == "d") {
		return decompressor(argv[2], argv[3]);
	}
	else {
		std::cerr << "Error: first argument is not acceptable\n";
		return EXIT_FAILURE;
	}
}
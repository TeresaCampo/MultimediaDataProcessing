#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <print>

#include <iterator>
#include <string>
#include <map>
#include <vector>
#include <bitset>

#include <algorithm>


class codesBuilder {
	std::map<char, std::pair<int,uint32_t>> codesTable_;	//byte, number bit of code, code in binary

public:
	codesBuilder() {}
	~codesBuilder() {}

	void insert_symbol(char symbol) {
		codesTable_[symbol] = {0,0};
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
			if (isprint(el.first)) {
				//std::cout << el.first << "->" << std::bitset<32>(el.second.second) << "\n";
				std::println("{:c}->{:0{}b}", el.first, el.second.second, el.second.first);
			}
		}
	}
};

class bitwriter {
	uint32_t buffer_ = { 0 };
	size_t n_=0;
	std::ofstream& os_;

public:
	bitwriter(std::ofstream& os) : os_(os){};
public:
	~bitwriter() {
		flush();
	};

	std::ostream& flush(uint32_t bit = 0) {
		write_n_bits(0, 8 - n_);
		return os_;
	}

	void write_one_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;

		if (n_ == 8) {
			os_.write(reinterpret_cast<char *>(&buffer_), 1);
			n_ = 0;
			buffer_ = 0;
		}
	}

public:
	void write_n_bits(uint32_t bit, size_t nbit) {
		while(nbit-->0) {
			write_one_bit(bit >> nbit);
		}
	}
};

void insertMergedAsFirst(std::vector<std::pair<std::vector<char>, float>>& f, std::pair<std::vector<char>, float> merged){
	auto it = f.begin();
	for (int i = 0; i < f.size() ; i++) {
		if (merged.second < f[i].second) {
			f.insert(it+i+1, merged);
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
	std::ifstream is(input, std::ios::binary);
	if (!is) {
		std::cerr << "Error: fail opening " << input << "\n";
		return EXIT_FAILURE;
	}

	std::ofstream os(output, std::ios::binary);
	if (!os) {
		std::cerr << "Error: fail opening " << output << "\n";
		return EXIT_FAILURE;
	}

	// count frequencies
	std::map<char, float> f_tmp;
	char val;
	uint32_t counter=0;
	while (!is.get(val).eof()) {
		f_tmp[val] += 1;
		++counter;
	}

	//create vector to order and merge iteratively (symbols-frequencies)
	//create codesBuilder to build huffman 
	std::vector<std::pair<std::vector<char>, float>> f;
	codesBuilder cbuilder;
	for ( auto& element : f_tmp) {
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
		insertMergedAsLast(f,merge);
	}

	//print table in file
	cbuilder.DEBUG_printCodesTable();
	std::map<char, std::pair<int, uint32_t>> codesTable = cbuilder.getCodes();

	//print HUFFMAN1, number table rows and table
	os << "HUFFMAN1";

	bitwriter bw(os);
	bw.write_n_bits(codesTable.size(),8);

	for (const auto& el : codesTable) {
		bw.write_n_bits(el.first,8);
		bw.write_n_bits(el.second.first, 5);
		bw.write_n_bits(el.second.second, el.second.first);
	}

	//print number of symbols in the original file(BIG ENDIAN)
	bw.write_n_bits(counter, 32);

	//read input file from beginning again
	is.clear();
	is.seekg(0);

	std::pair<int, uint32_t> symbol_code;
	while (!is.get(val).eof()) {
		symbol_code = codesTable[val];
		bw.write_n_bits(symbol_code.second, symbol_code.first);
	}
	return EXIT_SUCCESS;
}

//-----------------------------------decompressor-----------------------------------------------------------
class bitreader {
	int n_ = 0;
	uint8_t buffer_ = 0;
	std::ifstream& is_;

	uint32_t read_one_bit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*> (&buffer_), 1);
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::ifstream& is) : is_(is) {};
	~bitreader() {};
	
	std::ifstream& read_n_bits(uint32_t& val, size_t nbits) {
		val = 0;
		while (nbits-- > 0) {
			val = (val << 1) | read_one_bit();
		}
		return is_;
	}
};

int decompressor(std::string input, std::string output) {
	std::ifstream is(input, std::ios::binary);
	if (!is) {
		std::cerr << "Error: fail opening " << input << "\n";
		return EXIT_FAILURE;
	}

	std::ofstream os(output, std::ios::binary);
	if (!os) {
		std::cerr << "Error: fail opening " << output << "\n";
		return EXIT_FAILURE;
	}

	//read header
	std::string header(8, ' ');
	is.read(&header[0], 8);
	if (header != std::string("HUFFMAN1")) {
		std::cerr << "Error: file is not encoded with huffman1 algorithm";
		return EXIT_FAILURE;
	}

	//read number of huffman code-symbol table's rows
	uint32_t nTableEntries = 0;
	is.read(reinterpret_cast<char*>(& nTableEntries), 1);
	if (nTableEntries == 0) nTableEntries = 256;

	//read huffman code-symbol table
	std::map<int,std::vector< std::pair<uint32_t, char>>> codesDict;
	uint32_t symbol;
	uint32_t code;
	uint32_t codeLenght;

	bitreader br(is);
	while (nTableEntries-- > 0) {
		br.read_n_bits(symbol, 8);
		br.read_n_bits(codeLenght, 5);
		br.read_n_bits(code, codeLenght);
		
		codesDict[codeLenght].push_back({ code, symbol });
	}

	//read number of symbols
	uint32_t nSymbols = 0;
	br.read_n_bits(nSymbols, 32);

	//decode data using huffman table
	bitwriter bw(os);
	uint32_t nextBit;
	uint32_t codeRead = 0;
	char n = 0;
	
	//read and decode symbols
	while (!br.read_n_bits(nextBit, 1).eof() and nSymbols>0) {
		++n;
		codeRead = codeRead << 1 | (nextBit & 1);

		auto it = codesDict.find(n);
		if (it!=codesDict.end()) {
			for (const auto& el : it->second) {
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
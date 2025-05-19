#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <iterator>

#include <map>
#include <vector>
#include <string>

#include <algorithm>

struct bitwriter {
	std::ofstream& os_;
	uint32_t buffer_ = 0;
	uint32_t n_ = 0;

	bitwriter(std::ofstream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}
	void flush() {
		write_n_bits(0, 8 - n_);
	}
	void write_one_bit(const uint32_t& bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;

		if (n_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 0;
			buffer_ = 0;
		}
	}

	void write_n_bits(const uint32_t& val, uint32_t n) {
		while (n-- > 0) {
			write_one_bit(val >> n);
		}
	}
};

struct bitreader {
	std::ifstream& is_;
	uint32_t buffer_ = 0;
	uint32_t n_ = 0;

	bitreader(std::ifstream& is) :is_(is) {}
	uint32_t read_one_bit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

	std::ifstream& read_n_bits(uint32_t& val, uint32_t n) {
		val = 0;
		while (n-- > 0) {
			val = (val << 1) | read_one_bit();
		}
		return is_;
	}

};

struct h_code {
	size_t frequency_ = 0;
	uint32_t code_ = 0;
	uint32_t len_ = 0;
	char sym_;

	h_code() {}
	h_code(const char& sym) : sym_(sym), frequency_(1) {}
	h_code(uint32_t code, uint32_t len) :code_(code), len_(len) {}
	h_code(uint32_t code, uint32_t len, char sym) :code_(code), len_(len), sym_(sym) {}


	void update_code(char bit) {
		code_ = code_ | (bit & 1) << len_;
		len_++;
	}
};

void frequency_counter(std::ifstream& is, std::map<char, std::unique_ptr<h_code>>& f, uint32_t& total_counter) {
	char sym;
	while (!is.get(sym).eof()) {
		total_counter++;
		if (f.find(sym) == f.end()) {
			f[sym] = std::make_unique<h_code>(sym);
		}
		else {
			f[sym]->frequency_++;
		}
	}
}

struct h_code_vector {
	std::vector<std::unique_ptr<h_code>> v_;

	h_code_vector(std::unique_ptr<h_code>& first) {
		v_.push_back(std::move(first));
	}

	h_code_vector(h_code_vector& a1, h_code_vector& a2) {
		v_.reserve(a1.v_.size() + a2.v_.size());
		std::move(std::make_move_iterator(begin(a1.v_)),
			std::make_move_iterator(end(a1.v_)),
			std::back_inserter(v_));
		std::move(std::make_move_iterator(begin(a2.v_)),
			std::make_move_iterator(end(a2.v_)),
			std::back_inserter(v_));
	}

};

int huffman_compressor(std::ifstream& is, std::ofstream& os) {
	using std::pair;

	//step 1--> calculate frequency
	std::map<char, std::unique_ptr<h_code>> f;
	uint32_t total_counter = 0;

	frequency_counter(is, f, total_counter);

	//step 2-->ordered vector (frequency-symbols)
	std::vector<pair<size_t, h_code_vector>> v;
	for (auto& [sym, code] : f) {
		v.push_back({ code->frequency_, h_code_vector(code) });
	}

	std::sort(begin(v), end(v), [](const pair<size_t, h_code_vector>& a1, const pair<size_t, h_code_vector>& a2) {
		if (a1.first == a2.first)   return a1.second.v_[0]->sym_ > a2.second.v_[0]->sym_;
		else						return a1.first > a2.first;
		});

	//step 3 -->huffman algorithm to create code table
	while (v.size() > 1) {
		pair<size_t, h_code_vector> b1 = std::move(v.back());
		v.pop_back();
		pair<size_t, h_code_vector> b2 = std::move(v.back());
		v.pop_back();

		for (auto& code : b1.second.v_) {
			code->update_code(0);
		}
		for (auto& code : b2.second.v_) {
			code->update_code(1);
		}

		pair<size_t, h_code_vector> merge = { b1.first + b2.first, std::move(h_code_vector(b1.second, b2.second)) };

		auto it = std::lower_bound(begin(v), end(v), merge, [](const pair<size_t, h_code_vector>& a1, const pair<size_t, h_code_vector>& a2) {return a1.first > a2.first; });
		v.insert(it, std::move(merge));
	}

	//step 4 -->create huffman table for encoding (sym-code len-code)
	//step 4bis -->create canonical huffman table for printing
	std::map<char, h_code> table;
	std::vector<pair<char, uint32_t>> table_print;


	// 1. Raccogli simboli e lunghezze
	for (const auto& code : v[0].second.v_) {
		table_print.push_back({ code->sym_, code->len_ });
	}

	// 2. Ordina per lunghezza crescente, poi simbolo crescente
	std::sort(begin(table_print), end(table_print), [](const pair<char, uint32_t>& a, const pair<char, uint32_t>& b) {
		if (a.second == b.second) return a.first < b.first;
		return a.second < b.second;
		});

	// 3. Genera codici canonici
	std::map<char, h_code> table_c;
	uint32_t current_code = 0;
	uint32_t current_len = table_print[0].second;

	for (const auto& [sym, len] : table_print) {
		while (current_len < len) {
			current_code <<= 1;
			current_len++;
		}
		table_c[sym] = h_code(current_code, len);
		current_code++;
	}


	//step 5 -->print huffman encoding info
	os << "HUFFMAN2";
	bitwriter bw(os);

	bw.write_n_bits(table_c.size(), 8);

	for (const auto& sym : table_print) {
		bw.write_n_bits(sym.first, 8);
		bw.write_n_bits(sym.second, 5);
	}

	bw.write_n_bits(total_counter, 32);

	is.clear();
	is.seekg(0);
	while (total_counter-- > 0) {
		char val = is.get();
		bw.write_n_bits(table_c[val].code_, table_c[val].len_);
	}

	return EXIT_SUCCESS;
}

int huffman_decompressor(std::ifstream& is, std::ofstream& os) {
	//Step 1--> check header
	std::string header(8, ' ');
	is.read(header.data(), 8);
	if (header != "HUFFMAN2") {
		std::cerr << "Error: file is not compressed with canonical huffman code\n.";
		return EXIT_FAILURE;
	}

	//step 2--> check numer of symbols in huffman table
	uint32_t numSymbols = 0;
	is.read(reinterpret_cast<char*>(&numSymbols), 1);
	if (numSymbols == 0) numSymbols = 256;

	//step 3--> create huffman table with canonical algorithm
	std::map<uint32_t, std::vector<h_code>> table;
	h_code current_code;
	bitreader br(is);

	for (int i = 0; i < numSymbols; i++) {
		uint32_t sym = 0;
		uint32_t len = 0;
		br.read_n_bits(sym, 8);
		br.read_n_bits(len, 5);

		while (current_code.len_ < len) {
			current_code.code_ = current_code.code_ << 1;
			current_code.len_++;
		}
		auto it = table.find(len);
		if (it != table.end()) {
			it->second.push_back({ current_code.code_, current_code.len_, static_cast<char>(sym) });
		}
		else {
			std::vector<h_code> codes;
			codes.push_back({ current_code.code_, current_code.len_,  static_cast<char>(sym) });
			table[len] = codes;
		}

		current_code.code_ = current_code.code_ | 1;
	}

	//step 4-->read number of symbols in the file
	uint32_t total_counter = 0;
	br.read_n_bits(total_counter, 32);

	//step 5 -->decode file
	bitwriter bw(os);
	current_code.code_ = 0;
	current_code.len_ = 0;

	while (total_counter > 0) {
		current_code.code_ = (current_code.code_ << 1) | (br.read_one_bit() & 1);
		current_code.len_++;

		auto it = table.find(current_code.len_);
		if (it != table.end()) {
			for (const auto& code : it->second) {
				if (code.code_ == current_code.code_) {
					bw.write_n_bits(code.sym_, 8);
					current_code.code_ = 0;
					current_code.len_ = 0;
					--total_counter;
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

	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		std::cerr << "Error: fail opening input file" << argv[2] << "\n.";
		return EXIT_FAILURE;
	}

	std::ofstream os(argv[3], std::ios::binary);
	if (!os) {
		std::cerr << "Error: fail opening input file" << argv[3] << "\n.";
		return EXIT_FAILURE;
	}

	if (std::string(argv[1]) == "c") {
		return huffman_compressor(is, os);
	}
	if (std::string(argv[1]) == "d") {
		return huffman_decompressor(is, os);
	}
	else {
		std::cerr << "Error: first argument is not acceptable\n";
		return EXIT_FAILURE;
	}
}
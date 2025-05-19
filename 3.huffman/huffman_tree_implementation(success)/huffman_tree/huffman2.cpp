#include <iostream>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <algorithm>
//#include <print>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
//-------------------compressor-------------------
struct bitwriter {
	std::ofstream& os_;
	char buffer_ = 0;
	int n_ = 0;

	bitwriter(std::ofstream& os) : os_(os) {};
	~bitwriter() {
		flush();
	}

	void write_one_bit(int val) {
		buffer_ = (buffer_ << 1) | (val & 1);
		n_++;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
			buffer_ = 0;
		}
	}

	void write_n_bit(int val, int nbit) {
		while (nbit-- > 0) {
			write_one_bit(val >> nbit);
		}
	}

	void flush() {
		while (n_ > 0) {
			write_one_bit(0);
		}
	}
};
struct node {
	char symbol_ = 0;
	int code_ = 0;
	int len_ = 0;
	int freq_ = 0;
	node* right_ = nullptr;
	node* left_ = nullptr;

	node(char symbol, int freq) : symbol_(symbol), freq_(freq) {};
	node(char symbol, int len, int code) : symbol_(symbol), len_(len), code_(code) {};

	node(node* n1, node* n2) : right_(n1), left_(n2), freq_(n1->freq_ + n2->freq_) {};
};
struct huffman_compressor {
	std::unordered_map<char, int> f_;
	std::vector<std::unique_ptr<node>> mem_;
	std::map<char, node*> huffman_table_;
	std::vector<node*> ordered_huffman_table_;
	std::vector<char>& data_;
	huffman_compressor(std::vector<char>& data): data_(data) {};

	void frequency(std::vector<char> data) {
		for (const auto& el : data) {
			f_[el]++;
		}
	}
	void make_codes(node* n, int len) {
		if (n->left_ == nullptr) {
			n->len_ = len;
			huffman_table_[n->symbol_] = n;
		}
		else {
			make_codes(n->left_, len + 1);
			make_codes(n->right_, len + 1);
		}
	}
	
	/*void DEBUG_print_huffman_table() {
		for (const auto& node : ordered_huffman_table_) {
			std::println("{:c} {:0{}b}", node->symbol_, node->code_, node->len_);
		}
	}*/
	void create_huffman_table() {
		//count frequency of each symbol
		frequency(data_);

		//create nodes and order in a vector
		std::vector<node*> v;
		for (const auto& [symbol, freq] : f_) {
			mem_.push_back(std::make_unique<node>(symbol, freq));
			v.push_back(mem_.back().get());
		}
		std::sort(begin(v), end(v), [](const node* a1, const node* a2) {
			return a1->freq_ > a2->freq_;
			});
		//huffman algorithm to find lenghts
		while (v.size() > 1) {
			auto l1 = v.back();
			v.pop_back();
			auto l2 = v.back();
			v.pop_back();

			mem_.push_back(std::make_unique<node>(l1, l2));
			auto it = std::lower_bound(begin(v), end(v), mem_.back().get(), [](const node* a1, const node* a2) {
				return a1->freq_ > a2->freq_;
				});
			v.insert(it, mem_.back().get());
		}
		//build symbol, len with recursion
		auto root = v.back();
		v.pop_back();
		make_codes(root, 0);

		//build codes with canonical method
		for (const auto [symbol, node] : huffman_table_) {
			ordered_huffman_table_.push_back(node);
		}
		std::sort(begin(ordered_huffman_table_), end(ordered_huffman_table_), [](const node* a1, const node* a2) {
			if (a1->len_ == a2->len_) return a1->symbol_ < a2->symbol_;
			else return a1->len_ < a2->len_;
			});

		int code = 0;
		int len = 0;
		for (auto node : ordered_huffman_table_) {
			while (len < node->len_) {
				code = code << 1;
				len++;
			}
			node->code_ = code;
			code += 1;
		}
	}
	void compress_data(std::ofstream& os) {
		os << "HUFFMAN2";
		os.put(static_cast<char>(huffman_table_.size()));
		
		bitwriter bw(os);
		for (const auto& node : ordered_huffman_table_) {
			bw.write_n_bit(node->symbol_, 8);
			bw.write_n_bit(node->len_, 5);
		}

		bw.write_n_bit(static_cast<int>(data_.size()), 32);

		for (const auto& symbol : data_) {
			bw.write_n_bit(huffman_table_[symbol]->code_, huffman_table_[symbol]->len_);
		}
	}
};
int compressor(std::ifstream& is, std::ofstream& os) {
	//read all file in a vector
	is.seekg(0, std::ios::end);
	size_t file_len = is.tellg();

	std::vector<char> data(file_len);
	is.seekg(0);
	is.read(data.data(), file_len);

	//huffman encoding
	huffman_compressor h(data);
	h.create_huffman_table();
	h.compress_data(os);
	
	return EXIT_SUCCESS;
}

//-------------------decompressor-------------------
struct bitreader {
	std::ifstream& is_;
	char buffer_ = 0;
	int n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};

	int read_one_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

	std::ifstream& read_n_bit(int nbit, int& val) {
		val = 0;
		while (nbit-- > 0) {
			val = (val << 1) | read_one_bit();
		}
		return is_;
	}
};

struct huffman_decompressor {
	std::vector<node*> huffman_table_;
	std::ifstream& is_;
	std::ofstream& os_;
	std::vector<std::unique_ptr<node>> mem_;
	huffman_decompressor(std::ifstream& is, std::ofstream& os): is_(is), os_(os) {};

	int decode() {
		//read header
		std::string header(8, ' ');
		is_.read(&header[0], 8);
		if (header != "HUFFMAN2") {
			std::cerr << "ERROR: file is not encoded with canonical huffman.\n";
			return EXIT_FAILURE;
		}

		//create huffman table
		int num_symbols = is_.get();
		if (num_symbols == 0) num_symbols = 256;

		bitreader br(is_);
		int code_builder = 0;
		int len_code_builder = 0;
		while (num_symbols-->0) {
			int symbol;
			int len = 0;

			br.read_n_bit(8, symbol);
			br.read_n_bit(5, len);
			while (len_code_builder < len) {
				code_builder = code_builder << 1;
				len_code_builder++;
			}
			
			mem_.push_back(std::make_unique<node>(static_cast<char>(symbol), len, code_builder));
			huffman_table_.push_back(mem_.back().get());
			code_builder += 1;
		}

		//decode data
		bitwriter bw(os_);

		int data_len = 0;
		br.read_n_bit(32, data_len);

		int code_reader = 0;
		int len_code_reader = 0;
		while (data_len-- > 0) {
			for (const auto& symbol : huffman_table_) {
				while (len_code_reader < symbol->len_) {
					code_reader = code_reader << 1 | br.read_one_bit();
					len_code_reader++;
				}
				if (symbol->len_ == len_code_reader and symbol->code_ == code_reader) {
					bw.write_n_bit(symbol->symbol_, 8);
					len_code_reader = 0;
					code_reader = 0;
					break;
				}
			}
		}
		return EXIT_SUCCESS;
	}
};
int decompressor(std::ifstream& is, std::ofstream& os) {
	huffman_decompressor h(is, os);
	return h.decode();
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "ERROR: wrong number of input parameters.\n";
		return EXIT_FAILURE;
	}
	if (std::string(argv[1]) != "c" and std::string(argv[1]) != "d") {
		std::cerr << "ERROR: first argument should be 'c' or 'd'.\n";
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[2], std::ios::binary);
	if (is.fail()) {
		std::cerr << "ERROR: fail opening file " << argv[2] << ".\n";
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[3], std::ios::binary);
	if (os.fail()) {
		std::cerr << "ERROR: fail opening file " << argv[3] << ".\n";
		return EXIT_FAILURE;
	}

	if (std::string(argv[1]) == "c") {
		return compressor(is, os);
	}
	if (std::string(argv[1]) == "d") {
		return decompressor(is, os);
	}
}
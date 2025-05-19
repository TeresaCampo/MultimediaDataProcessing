#include <print>
#include <fstream>
#include <expected>
#include <format>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include <cmath>
#include <memory>
#include <algorithm>

struct bitwriter {
	std::ofstream& os_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	bitwriter(std::ofstream& os) : os_(os) {};
	~bitwriter() {
		while (n_ != 0) {
			write_one_bit(0);
		}
	}

	void write_one_bit(int val) {
		buffer_ = buffer_ << 1| (val & 1);
		n_ ++;

		if (n_ == 8) {
			os_.write(reinterpret_cast<char *>( & buffer_), 1);
			n_ = 0;
		}
	}

	void write_n_bits(int val, int n_bits) {
		while (n_bits--> 0) {
			write_one_bit((val >> n_bits) & 1);
		}
	}
};
struct bitreader {
	std::ifstream& is_;
	uint8_t buffer_ = 0;
	uint32_t n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};
	~bitreader(){}

	int read_one_bit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 8;
		}

		n_--;
		return (buffer_ >> n_) & 1;
	}

	int read_n_bits(int n_bits) {
		int val = 0;
		while (n_bits-- > 0) {
			val = (val << 1) | read_one_bit();
		}
		return val;
	}
};

template<typename T>
struct image {
	int r_, c_;
	std::vector<T> data_;

	image(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r * c);
	}

	int width() { return c_; }
	int height() { return r_; }
	size_t size() { return c_ * r_; }

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	char* rawdata() const {
		return reinterpret_cast<const char*>(data_.data());
	}
	size_t rawsize() {
		return data_.size() * sizeof(T);
	}
	T& operator()(int r, int c) {
		return data_[r * c_ + c];
	}

	std::vector<T>& data() {
		return data_;
	}
};
struct node {
	int16_t symbol_ = 0;
	uint32_t len_code_ = 0;
	uint32_t code_ = 0;
	node* r_ = nullptr;
	node* l_ = nullptr;

	size_t f_ = 0;

	node(int16_t symbol, size_t f) : symbol_(symbol), f_(f) {};
	node(int16_t symbol, uint32_t code, uint32_t len_code) : symbol_(symbol), code_(code), len_code_(len_code) {};


	node(node* r, node* l, size_t f) :r_(r), l_(l), f_(f) {};
};
void recursive(node * n, int len, std::vector<node*>& leaves) {
	if (n->l_ != nullptr) recursive(n->l_, len + 1, leaves);
	if (n->r_ != nullptr) recursive(n->r_, len + 1, leaves);
	if (n->l_ == nullptr and n->r_ == nullptr) {
		n->len_code_ = len;		//set len_code in leaves
		leaves.push_back(n);
	}
}

struct huffman {
	std::vector< std::unique_ptr<node> > mem_;
	std::vector<node*> leaves_;
	std::map<int16_t, node*> canonical_huffman_table_;
	

	void canonical_huffman(std::vector<int16_t>& data) {
		//calculate symbol frequencies
		std::unordered_map<int16_t, size_t> f;

		for (const auto& symbol : data) {
			f[symbol] += 1;
		}

		//create an ordered vector of nodes
		std::vector<node*> nodes;

		for (const auto& [symbol, frequency] : f) {
			mem_.push_back(std::make_unique<node>(symbol, frequency));
			nodes.push_back(mem_.back().get());
		}

		std::sort(begin(nodes), end(nodes), [](const auto& el1, const auto& el2) {
			return el1->f_ > el2->f_;
			}
		);

		//create tree
		while (nodes.size() > 1) {
			auto l1 = nodes.back();
			nodes.pop_back();

			auto l2 = nodes.back();
			nodes.pop_back();

			mem_.push_back(std::make_unique<node>(l1, l2, l1->f_ + l2->f_));
			auto new_node = mem_.back().get();
			auto it = std::lower_bound(begin(nodes), end(nodes), new_node, [](const auto& el1, const auto& el2) {
				return el1->f_ > el2->f_;
				});
			nodes.insert(it, new_node);
		}

		//create lengths
		recursive(nodes[0], 0, leaves_);

		std::sort(begin(leaves_), end(leaves_), [](const auto& el1, const auto& el2) {
				if (el1->len_code_ == el2->len_code_) {
					return el1->symbol_ < el2->symbol_;
				}
				else {
					return el1->len_code_ < el2->len_code_;
				}
			});

		//create codes
		uint32_t tmp = 0;
		uint32_t len_tmp = 1;

		for (auto& leaf : leaves_) {
			while (len_tmp < leaf->len_code_) {
				tmp = tmp << 1;
				len_tmp++;
			}

			leaf->code_ = tmp;
			tmp += 1;
		}

		//create canonical huffman table
		for (const auto& leaf : leaves_) {
			canonical_huffman_table_[leaf->symbol_] = leaf;
		}
	}

	void print_huffman_table() {
		std::ofstream compressione_huffman_table_file("c_ht.txt");
		for (const auto& el : leaves_) {
			compressione_huffman_table_file << std::format("{} ->\t{:0{}b}\n", el->symbol_, el->code_, el->len_code_);
		}
	}
};

std::expected<image<uint8_t>, std::string>
read_PAM_image(std::ifstream& is) {
	//read header
	std::string field, value;
	int r = -1;
	int c = -1; 
	int d = -1;
	int maxval = -1;

	is >> field;
	if (field != "P7") return std::unexpected("input file format is not PAM");

	while (1) {
		is >> field;
		if (field == "ENDHDR") {
			is.get();
			break;
		}
		if (field[0] == '#') {
			std::string dummy;
			std::getline(is, dummy);
			continue;
		}

		is >> value;
		if (field == "HEIGHT")	r = std::stoi(value);
		if (field == "WIDTH")	c = std::stoi(value);
		if (field == "MAXVAL") { 
			maxval = std::stoi(value);
			if (maxval > 255) return std::unexpected("input image has maxval " + std::to_string(maxval) + ", but it should be lower to 256.");
		}
		if (field == "DEPTH") {
			d = std::stoi(value);
			if (d != 1) return std::unexpected("input image has depth " + std::to_string(d) + ", but should have d =1.");
		}
	}
	if ((maxval < 0) or (c * r * d) < 0)	return std::unexpected("not enaugh info to decompress .pam file.");
	
	//create image
	image<uint8_t> img(r, c);
	is.read(img.rawdata(), img.rawsize());
	return img;
}
void write_PAM_image(std::ofstream& os, image<uint8_t> img) {
	os << "P7\n";
	os << "WIDTH " << img.width() << "\n";
	os << "HEIGHT " << img.height() << "\n";
	os << "DEPTH " << "1" << "\n";
	os << "MAXVAL " << "255" << "\n";
	os << "TUPLTYPE " << "GRAYSCALE" << "\n";
	os << "ENDHDR\n";
	os.write(img.rawdata(), img.rawsize());
}
void calculate_difference_matrix(image<uint8_t>& img, image<int16_t>& d_img){
	for (int r = 0; r < img.height(); r++) {
		for (int c = 0; c < img.width(); c++) {
			//element in position (0,0)
			if ((c == 0) and (r == 0)) d_img(r, c) = img(r, c);

			//element in position (x,y)
			else if (c != 0) d_img(r, c) = (img(r, c) - img(r, c - 1));

			//element in position (0,y)
			else if ((c == 0) and (r != 0)) d_img(r, c) = (img(r, c) - img(r - 1, c));
		}
	}
}
void reconstruct_difference_matrix(image<int16_t>& d_img, image<uint8_t>& img) {
	for (int r = 0; r < d_img.height(); r++) {
		for (int c = 0; c < d_img.width(); c++) {
			//element in position (0,0)
			if ((c == 0) and (r == 0)) img(r, c) = static_cast<uint8_t>(d_img(r, c));

			//element in position (x,y)
			else if (c != 0) img(r, c) = (d_img(r, c) + img(r, c - 1));

			//element in position (0,y)
			else if ((c == 0) and (r != 0)) img(r, c) = (d_img(r, c) + img(r - 1, c));
		}
	}
}
void debug_difference_matrix(image<int16_t>& d_img, std::string output_file_name) {
	image<uint8_t> normalized_d_img(d_img.height(), d_img.width());

	for (int r = 0; r < d_img.height(); r++) {
		for (int c = 0; c < d_img.width(); c++) {
			normalized_d_img(r, c) = static_cast<uint8_t>(std::floor(d_img(r, c) / 2) +128);
		}
	}

	std::ofstream os_test(output_file_name, std::ios::binary);
	if (!os_test) {
		std::println("Error: output file {} can't be opned.", output_file_name);
	}

	os_test << "P7\n";
	os_test << "WIDTH " << d_img.width() << "\n";
	os_test << "HEIGHT " << d_img.height() << "\n";
	os_test << "DEPTH " << "1" << "\n";
	os_test << "MAXVAL " << "255" << "\n";
	os_test << "TUPLTYPE " << "GRAYSCALE" << "\n";
	os_test << "ENDHDR\n";
	os_test.write(normalized_d_img.rawdata(), normalized_d_img.rawsize());
}

std::expected<bool, std::string>
compression(std::ifstream& is, std::ofstream& os) {
	//read PAM image, maxval <=255 and grayscale
	auto res = read_PAM_image(is);
	if (!res) {
		return std::unexpected(res.error());
	}

	image<uint8_t> img = res.value();

	//create difference matrix
	image<int16_t> d_img(img.height(), img.width());
	for (int r = 0; r < img.height(); ++r) {
		for (int c = 0; c < img.width(); ++c) {
			if ((c == 0) and (r == 0)) {
				d_img(r, c) = img(r, c);
			}
			else if (c!=0){
				d_img(r, c) = img(r, c) - img(r, c-1);
			}
			else if ((c == 0) and (r!=0)) {
				d_img(r, c) = img(r, c) - img(r -1, c);
			}
		}
	}
	debug_difference_matrix(d_img, "c_difference_matrix_test.pam");
	bitwriter bw(os);

	//write huffman table
	os << "HUFFDIFF";
	int w = d_img.width();
	int h = d_img.height();
	os.write(reinterpret_cast<char*>(&w), sizeof(int));
	os.write(reinterpret_cast<char*>(&h), sizeof(int));

	huffman huffman;
	huffman.canonical_huffman(d_img.data());

	std::vector<node*> cht_leaves = huffman.leaves_;
	bw.write_n_bits(static_cast<uint32_t>(cht_leaves.size()), 9);

	for (const auto& leaf : cht_leaves) {
		auto uns_symbol = static_cast<uint32_t>(leaf->symbol_);
		auto sing_symbol = leaf->symbol_;
		bw.write_n_bits(static_cast<uint32_t>(leaf->symbol_), 9);
		bw.write_n_bits(leaf->len_code_, 5);
	}

	//create huffman table dictionary and write encoded data
	std::map<int16_t, node*> cht = huffman.canonical_huffman_table_;

	for (const auto& pixel : d_img.data()) {
		bw.write_n_bits(cht[pixel]->code_, cht[pixel]->len_code_);
	}

	return EXIT_SUCCESS;
}

std::expected<bool, std::string>
decompression(std::ifstream& is, std::ofstream& os) {
	bitreader br(is);
	//check huffdiff header
	std::string header(8, ' ');
	is.read(header.data(), 8);
	if (header != std::string("HUFFDIFF")) {
		return std::unexpected("wrong huffdiff header.");
	}

	int w = 0;
	int h = 0;
	is.read(reinterpret_cast<char*>(&w), 4);
	is.read(reinterpret_cast<char*>(&h), 4);
	image<int16_t> d_img(h, w);

	uint32_t num_elem = br.read_n_bits(9);

	//read canonical huffman table and construct the canonical codes
	std::map<uint32_t, std::vector<node>> cht;
	uint32_t tmp = 0;
	uint32_t len_tmp = 1;
	while (num_elem--> 0) {
		//read symbol(make it unsigned) and length
		uint16_t symbol = br.read_n_bits(9);
		int16_t signed_symbol = 0;

		if ((symbol >> 8) == 1){
			signed_symbol = (127 << 9) | symbol;
		}
		else {
			signed_symbol = symbol;
		}
		uint32_t len_code = br.read_n_bits(5);

		//create canonical code
		while (len_tmp < len_code) {
			tmp = tmp << 1;
			len_tmp++;
		}
		cht[len_code].push_back(node(signed_symbol, tmp, len_code));
		tmp += 1;
	}

	//read and decode data
	std::vector<int16_t>& d_img_data = d_img.data();
	size_t pixel_read = 0;
	tmp = 0;
	len_tmp = 0;

	while (!is.eof() and pixel_read<d_img.size()) {
		tmp = tmp << 1 | br.read_one_bit();
		len_tmp++;

		for (const auto& leaf : cht[len_tmp]) {
			if (leaf.code_ == tmp) {
				d_img_data[pixel_read] = leaf.symbol_;
				tmp = 0;
				len_tmp = 0;
				pixel_read++;

				break;
			}
		}
	}
	debug_difference_matrix(d_img, "d_difference_matrix_test.pam");

	//reconstruct orignal image and write it in a PAM file
	image<uint8_t> img(d_img.height(), d_img.width());
	reconstruct_difference_matrix(d_img, img);
	write_PAM_image(os, img);

	return EXIT_SUCCESS;
}



int main(int argc, char** argv) {
	
	if (argc != 4) {
		std::println("Error: program accepts 3 parameters, not {}.", argc-1);
	}

	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		std::println("Error: input file {} can't be opened.", argv[2]);
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[3], std::ios::binary);
	if (!os) {
		std::println("Error: output file {} can't be opned.", argv[3]);
	}

	// compression
	if (std::string(argv[1]) == "c") {
		auto res = compression(is, os);
		
		if (res) return EXIT_SUCCESS;
		else {
			std::println("Error: {}", res.error());
			return EXIT_FAILURE;
		}
	}
	//decompression
	if (std::string(argv[1]) == "d") {
		auto res = decompression(is, os);

		if (res) return EXIT_SUCCESS;
		else {
			std::println("Error: {}", res.error());
			return EXIT_FAILURE;
		}
	}
}
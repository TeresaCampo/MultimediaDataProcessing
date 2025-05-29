#include <cstdio>
#include <print>
#include <fstream>

#include <map>
#include <string>
#include <vector>


struct bitreader {
	std::ifstream& is_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};

	uint32_t read_one_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return buffer_ >> n_ & 1;
	}

	void read_n_bits(uint64_t& val, uint8_t nbits) {
		while (nbits-- > 0) {
			val = val << 1 | read_one_bit();
		}
	}
};
uint64_t read_size(std::ifstream& is) {
	bitreader br(is);

	//number of bytes reserved for EBML ID
	uint32_t c = 1;
	while (br.read_one_bit() != 1) {
		c++;
	}

	uint64_t size = 1;
	br.read_n_bits(size, (8 - c) + (c - 1) * 8);
	return size;
}
uint32_t read_ID(std::ifstream& is) {
	bitreader br(is);

	//number of bytes reserved for EBML ID
	uint32_t c = 1;
	while (br.read_one_bit() != 1) {
		c++;
	}

	uint64_t id = 1;
	br.read_n_bits(id, (8 - c) + (c - 1) * 8);
	return static_cast<uint32_t>(id);
}

struct EBML_element {
	std::ifstream& is_;
	uint32_t id_ = 0;
	uint64_t size_ = 0;
	std::vector<uint8_t> data_;

	std::string name_;
	std::string type_;
	EBML_element(std::string)

	EBML_element(std::ifstream& is): is_(is) {
		id_= read_ID(is);
		size_ = read_size(is);

		data_.resize(size_);
		is.read(reinterpret_cast<char*>(data_.data()), size_);
	}
};
std::map<uint32_t, EBML_element> EBML_table;

int read_header(std::ifstream& is) {
	EBML_element el(is);


	return EXIT_SUCCESS;

}

int load_EBML_schema(std::ifstream& is) {
	while (!is.eof()) {
		std::string dummy(200, ' ');
		std::getline(is, dummy, '<');

		std::string check(200, ' ');
		std::string element_line(200, ' ');
		std::getline(is, check, ' ');
		if (check == "element") {
			std::getline(is, element_line, '>');

		}
		//check if following is element
	}
	return EXIT_SUCCESS;
}


int main(int argc, char **argv) {
	if (argc != 2) {
		std::println("Error: program accepts 1 parameter, not {}.", argc - 1);
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}.", argv[1]);
		return EXIT_FAILURE;
	}

	std::ifstream is_table("ebml_matroska.xml", std::ios::binary);
	if (!is_table) {
		std::println("Error: fail opening input file {}.","ebml_matroska.xml" );
		return EXIT_FAILURE;
	}
	load_EBML_schema(is_table);
	//read header
	read_header(is);
}
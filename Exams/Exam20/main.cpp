#include <cstdio>
#include <print>
#include <fstream>

#include <string>
#include <vector>

uint32_t last_distance = 0;

struct element {
	std::vector<uint8_t> code_;
	std::vector<uint8_t> literals_;
	std::string type_;

	uint8_t nbytes_literals = 0;
	uint8_t match_length = 0;
	uint32_t match_distance = 0;

	element(std::string type, uint8_t b) {
		code_.push_back(b);
		type_ = type;
	}

	void extract_info() {
		nbytes_literals = static_cast<uint8_t>(literals_.size());
		
		if (type_ == "sml_d") {
			match_length = (code_[0] >> 3) & 0x07;
			match_length += 3;

			match_distance = code_[1];
			match_distance = match_distance | ((code_[0] & 0x07) << 8);
		}
		else if (type_ == "med_d") {
			match_length = code_[0] & 0x07;
			match_length = match_length << 2 | code_[1] & 0x03;
			
			match_length += 3;

			match_distance = code_[1] >> 2 & 0x3F;
			match_distance |= (code_[2] << 6);
		}
		else if (type_ == "lrg_d") {
			match_length = code_[0] >> 3 & 0x07;
			match_length += 3;

			match_distance = code_[2];
			match_distance = match_distance << 8 | code_[1];
		}
		else if (type_ == "pre_d") {
			match_length = code_[0] >> 3 & 0x07;
			match_length += 3;

			match_distance = last_distance;
		}
		else if (type_ == "sml_m") {
			match_length = code_[0] & 0x0F;

			match_distance = last_distance;
		}
		else if (type_ == "lrg_m") {
			match_length = code_[1];
			match_length += 16;

			match_distance = last_distance;
		}
		if (match_distance != 0) last_distance = match_distance;
		//last_distance = match_distance;
	}


};

void read_remaining_opcode(std::ifstream& is, element& e, uint8_t nbytes) {
	for (int i = nbytes; i > 0; i--) {
		e.code_.push_back(is.get());
	}
}
void read_literals(std::ifstream& is, element& e, uint8_t nbytes) {
	for (int i = nbytes; i > 0; i--) {
		e.literals_.push_back(is.get());
	}
}
element extract_element(std::ifstream& is) {
	uint8_t b = is.get();

	//udef
	if (b == 62 or b == 66 or b == 46 or b == 38 or b == 30) {
		return element("udef", b);
	}

	//eos
	if (b == 6) {
		element e = element("eos", b);
		read_remaining_opcode(is, e, 7);
		if (!is.eof()) {
			std::println("Error: problem with eos opcode, it's not eof");
		}
		return e;
	}

	//nop
	if (b == 22 or b == 14) {
		return element("nop", b);
	}

	//large literal
	if (b == 224) {
		element e = element("lrg_l", b);
		read_remaining_opcode(is, e, 1);

		uint8_t bytes_literals = e.code_[1]+16;
		read_literals(is, e, bytes_literals);

		return e;
	}

	//large match
	if (b == 240) {
		element e =element("lrg_m", b);
		e.code_.push_back(is.get());
		return e;
	}

	//small literal
	if ((b & 0xF0) == 224) {
		element e = element("sml_l", b);
		uint8_t bytes_literals = b & 0x0F;
		read_literals(is, e, bytes_literals);

		return e;
	}

	//small match
	if ((b & 0xF0) == 240) {
		return element("sml_m", b);
	}

	//medium distance
	if ((b & 0xE0) == 160) {
		element e = element("med_d", b);
		read_remaining_opcode(is, e, 2);

		uint8_t bytes_literals = b >> 3 & 0x03;
		read_literals(is, e, bytes_literals);
		return e;
	}

	//large distance
	if ((b & 0x07) == 7) {
		element e = element("lrg_d", b);
		read_remaining_opcode(is, e, 2);

		uint8_t bytes_literals = b >> 6 & 0x03;
		read_literals(is, e, bytes_literals);
		return e;
	}

	//previous distance
	if ((b & 0x07) == 6) {
		element e = element("pre_d", b);
		uint8_t bytes_literals = b >> 6 & 0x03;
		read_literals(is, e, bytes_literals);

		return e;
	}

	//small distance
	else {
		element e = element("sml_d", b);
		read_remaining_opcode(is, e, 1);

		uint8_t bytes_literals = b >> 6 & 0x03;
		read_literals(is, e, bytes_literals);
		return e;
	}
}

void write_run(std::vector<char>& decoded_data, element& e, int pos) {
	long initial_pos = pos + e.nbytes_literals - e.match_distance;
	if (initial_pos >= decoded_data.size() or initial_pos < 0) {
		auto a = 1;
	}
	for (int i = 0; i < e.match_length; i++) {
		decoded_data.push_back(decoded_data[initial_pos + i]);
	}
}

void write_literals(std::vector<char>& decoded_data, element e, int pos) {
	decoded_data.resize(decoded_data.size() + e.nbytes_literals);
	std::copy(begin(e.literals_), end(e.literals_), begin(decoded_data) + pos);
}
int decoder(std::ifstream& is, std::ofstream& os) {
	//read header
	std::string header(4, ' ');
	is.read(header.data(), 4);

	if (header != "bvxn") {
		std::println("Error: header is wrong");
		return EXIT_FAILURE;
	}

	int32_t out_bytes = 0;
	is.read(reinterpret_cast<char*>(&out_bytes), 4);
	int32_t in_bytes = 0;
	is.read(reinterpret_cast<char*>(&in_bytes), 4);

	std::vector<char> decoded_data;
	int pos = 0;
	while (1) {
		//read following element
		element e = extract_element(is);
		e.extract_info();

		if (e.type_ == "sml_d" or e.type_ == "med_d" or e.type_ == "lrg_d" or e.type_ == "pre_d") {
			write_literals(decoded_data, e, pos);
			write_run(decoded_data, e, pos);

			pos = pos + e.nbytes_literals + e.match_length;
		}
		else if (e.type_ == "sml_m" or e.type_ == "lrg_m") {
			write_run(decoded_data, e, pos);

			pos += e.match_length;;
		}
		else if (e.type_ == "sml_l" or e.type_ == "lrg_l") {
			write_literals(decoded_data, e, pos);

			pos += e.nbytes_literals;
		}
		else if (e.type_ == "eos") break;
		
		else if (decoded_data.size() > 1500) {
			auto a = 2;
		}

	}

	os.write(decoded_data.data(), decoded_data.size());
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}.", argv[1]);
	}
	std::ofstream os(argv[2], std::ios::binary);

	return decoder(is, os);
}
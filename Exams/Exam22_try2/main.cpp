#include <cstdio>
#include <print>
#include <fstream>
#include <algorithm>

#include <string>
#include <vector>
#include <map>

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
		return buffer_ >> (7-n_) & 1;
	}

	uint32_t read_n_bits(uint32_t nbits) {
		uint32_t val = 0;
		for (uint32_t i = 0; i<nbits; i++){
			val = val | read_one_bit() << i;
		}
		return val;
	}
};

bool check_header(std::ifstream& is) {
	std::string header(4, ' ');
	is.read(reinterpret_cast<char*>(header.data()), 4);
	if (header != "RIFF") {
		return false;
	}

	uint32_t total_payload = 0;
	is.read(reinterpret_cast<char*>(&total_payload), 4);

	is.read(reinterpret_cast<char*>(header.data()), 4);
	if (header != "WEBP") {
		return false;
	}

	is.read(reinterpret_cast<char*>(header.data()), 4);
	if (header != "VP8L") {
		return false;
	}

	uint32_t data_payload = 0;
	is.read(reinterpret_cast<char*>(&data_payload), 4);

	if (is.get() != 0x2f) {
		return false;
	}

	return true;
}
struct code {
	uint32_t len_ = 0;
	uint32_t code_ = 0;
	uint32_t symbol_ = 0;

	code(uint32_t symbol, uint32_t len) : symbol_(symbol), len_(len) {};
};
uint32_t read_huff_code(bitreader& br, std::map<uint32_t, std::vector<code>>& huff) {
	uint32_t tmp= 0;
	uint32_t tmp_len = 0;
	while (1) {
		tmp |= br.read_one_bit() << tmp_len;
		tmp_len++;

		auto it = huff.find(tmp_len);
		if (it!= end(huff)) {
			for (const auto& el : it->second) {
				if (el.code_ == tmp) return el.symbol_;
			}
		}
	}
}
void extract_normal_prefixcode(bitreader& br, uint8_t channel_number, std::map<uint32_t, std::vector<code>>& huff_channel) {
	uint32_t num_code_lengths = 4 + br.read_n_bits(4);

	//first huffman
	std::vector<code> codelen_codelen;
	for (int i = 0; i < 19; i++) {
		if (i < num_code_lengths)	codelen_codelen.emplace_back(i, br.read_n_bits(3));
		else						codelen_codelen.emplace_back(i, 0);
	}

	std::sort(begin(codelen_codelen), end(codelen_codelen), [](const code& e1, const code& e2) {
		if (e1.len_ == e2.len_) {
			return e2.symbol_ < e2.symbol_;
		}
		return e1.len_ < e2.len_;
		}
	);

	std::map<uint32_t, std::vector<code>> huff1;
	uint32_t tmp = 0;
	uint32_t tmp_len = 0;
	for (auto& el : codelen_codelen) {
		if (el.len_ == 0) continue;
		while (el.len_ > tmp_len) {
			tmp = tmp << 1;
			tmp_len++;
		}
		el.code_ = tmp;
		huff1[tmp_len].push_back(el);
		tmp++;
	}

	//enstablish max_symbol
	uint32_t maxval = 0;
	if (br.read_n_bits(1) == 0) {
		if (channel_number == 1) maxval = 256 + 24;
		if (channel_number == 5) maxval = 40;
		else maxval = 256;
	}
	else {
		int32_t len_nbits = 2 + 2 * br.read_n_bits(3);
		maxval = 2 + br.read_n_bits(len_nbits);
	}

	//read table lenghts
	std::vector<code> codelen;
	
	int i = 0;
	uint32_t previous = 0;
	while (i < maxval) {
		//read frist huff1 code
		uint32_t val = read_huff_code(br, huff1);
		if (val >= 1 and val <= 15) {
			codelen.emplace_back(i, val);
			previous = val;
			i++;
		}
		else if (val == 16) {
			uint32_t run = 3 + br.read_n_bits(2);
			while (run-- > 0) {
				uint32_t my_previous = (previous == 0) ? 8 : previous;
				codelen.emplace_back(i, previous);
				i++;
			}
		}
		else if (val == 17) {
			uint32_t run = 3 + br.read_n_bits(3);
			while (run-- > 0) {
				codelen.emplace_back(i, 0);
				i++;
			}
		}
		else if (val == 18) {
			uint32_t run = 11 + br.read_n_bits(7);
			while (run-- > 0) {
				codelen.emplace_back(i, 0);
				i++;
			}
		}
	}

	//sort huff2
	std::sort(begin(codelen), end(codelen), [](const code& e1, const code& e2) {
		if (e1.len_ == e2.len_) {
			return e2.symbol_ < e2.symbol_;
		}
		return e1.len_ < e2.len_;
		}
	);

	tmp = 0;
	tmp_len = 0;
	for (auto& el : codelen) {
		if (el.len_ == 0) continue;
		while (el.len_ > tmp_len) {
			tmp = tmp << 1;
			tmp_len++;
		}
		el.code_ = tmp;
		huff_channel[tmp_len].push_back(el);
		tmp++;
	}
}

int decode_webp(std::ifstream& is) {
	if (!check_header(is)) {
		std::println("Error: .webp file has wrong header.");
		return EXIT_FAILURE;
	}

	//width and height
	bitreader br(is);
	uint32_t width = br.read_n_bits(14) + 1;
	uint32_t height = br.read_n_bits(14) + 1;

	//additional info
	bool alpha_is_used = static_cast<bool>(br.read_n_bits(1));
	uint32_t version = br.read_n_bits(3);
	uint32_t transform_colorcache_metaprefix = br.read_n_bits(3);

	//one and only group of prefix codes (metaprefix bit = 0)


	

}

int main(int argc, char** argv) {
	//check parameters' number
	if (argc != 3) {
		std::println("Error: parameters shoudl be 2, not {}.", argc - 1);
		return EXIT_FAILURE;
	}

	//check files
	std::string ifile(argv[1]);
	if (ifile.substr(ifile.size() - 5, ifile.size()) != ".webp") {
		std::println("Error: input file shoul dbe .webp.");
		return EXIT_FAILURE;
	}
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}", ifile);
		return EXIT_FAILURE;
	}

	std::string ofile(argv[2]);
	if (ofile.substr(ofile.size() - 4, ofile.size()) != ".pam") {
		ofile.append(".pam");
	}
	std::ofstream os(ofile, std::ios::binary);

	decode_webp(is);
}
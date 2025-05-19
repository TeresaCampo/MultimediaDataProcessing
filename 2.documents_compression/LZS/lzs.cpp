#include "lzs.h"
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <bitset>

struct bitreader {
	std::istream& is_;
	char buffer_ = 0;
	int n_ = 0;

	bitreader(std::istream& is) : is_(is) {};
	int read_one_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}
	int read_n_bit(int nbit) {
		int val = 0;
		while (nbit-- > 0) {
			val = (val << 1) | read_one_bit();
		}
		return val;
	}
};
struct sliding_window {
	std::deque<char> w_;

	sliding_window() {}
	void slide_inserting_literals(char literal) {
		if (w_.size() == 2048) {
			w_.pop_back();
			w_.push_front(literal);
		}
		else {
			w_.push_front(literal);
		}
	}
	std::vector<char> slide_reading_offset(int offset, int len) {
		std::vector<char> r;
		offset = offset - 1; //set it as an index
		for (int i = offset; i > (offset - len); i--) {
			char literal = w_[offset];
			r.push_back(literal);
			slide_inserting_literals(literal);
		}

		return r;
	}
};

int find_len(bitreader& br) {
	int len = 0;
	int N = 0;
	//read 2 bits 
	// -> minor equal 2, len found
	//read 2 bits
	// -> minor equal 14, len found
	//flag N+1, read 2 bits
	// so on

	while (true) {
		len = 0;
		len = (len <<2) | br.read_n_bit(2);
		if ((len < 3) and (N==0)) break;

		len = (len << 2) | br.read_n_bit(2);
		if (len < 15) break;
		N++;
	}

	if ((N == 0) and (len < 3)) {
		return len + 2;
	}
	else {
		return len + ((N * 15) - 7);
	}
}
void lzs_decompress(std::istream& is, std::ostream& os) {
	bitreader br(is);
	sliding_window sw;
	
	while (!is.eof()) {
		int mode = br.read_one_bit();
		//1 -> eof OR offset
		//0 -> literal

		if (mode == 0) { //literal
			int literal = br.read_n_bit(8);

			os.put(static_cast<char>(literal));
			//std::cout << static_cast<char>(literal);
			sw.slide_inserting_literals(static_cast<char>(literal));
		}
		if (mode == 1) {
			//if follow is 1 -> eof OR offset minor than 128
			//both cases read next 7 bits
			int mode_offset = br.read_one_bit();
			if (mode_offset == 1) {
				int offset = br.read_n_bit(7);
				//eof
				if (offset == 0) {
					return;
				}

				//offset minor than 128
				int len = find_len(br);
				std::vector<char> literals = sw.slide_reading_offset(offset, len);
				os.write(literals.data(), literals.size());
				//std::cout.write(literals.data(), literals.size());
			}

			//if follow is 0 -> offset greater equal to 128
			//read next 11 bits
			if (mode_offset == 0) {
				int offset = br.read_n_bit(11);
				int len = find_len(br);
				std::vector<char> literals = sw.slide_reading_offset(offset, len);
				os.write(literals.data(), literals.size());
				//std::cout.write(literals.data(), literals.size());
			}
		}
	}
}
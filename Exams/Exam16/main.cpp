#include <cstdio>
#include <print>
#include <fstream>

#include <string>
#include <vector>

int read_header(std::ifstream& is) {
	uint8_t id1 = is.get();
	uint8_t id2 = is.get();
	uint8_t id3 = is.get();
	uint8_t id4 = is.get();
	if (id1 != 0x03 or id2 != 0x21 or id3 != 0x4C or id4 != 0x18) {
		std::println("Error: wrong magic number in header");
		return 1;
	}
	
	uint32_t len = 0;
	is.read(reinterpret_cast<char*>(&len), 4);

	id1 = is.get();
	id2 = is.get();
	id3 = is.get();
	id4 = is.get();
	if (id1 !=0 or id2 != 0 or id3 != 0 or id4 != 0x4D) {
		std::println("Error: wrong constant value in header");
		return 1;
	}
	return 0;
}
void write_run(std::vector<uint8_t>& data, uint16_t offset, long match_len) {
	int pos = data.size() - offset;
	for (long i = 0; i < match_len; i++) {
		data.push_back(data[pos+i]);
	}
}

void write_literals(std::vector<uint8_t>& data, std::vector<uint8_t> literals) {
	data.resize(data.size() + literals.size());
	std::copy(begin(literals), end(literals), end(data)- literals.size());
}

void decode_blocks(std::ifstream& is, std::ofstream& os) {
	//read each block
	std::vector<uint8_t> data;

	while (is.peek()!=EOF) {
		uint32_t block_len = 0;
		is.read(reinterpret_cast<char*>(&block_len), 4);


		std::vector<uint8_t> block(block_len);
		is.read(reinterpret_cast<char*>(block.data()), block_len);
		int pos = 0;

		while (1) {
			//read token
			uint8_t token = block[pos];
			pos++;

			//literal len
			long literal_len = token >> 4 & 0x0F;
			if (literal_len == 15) {
				uint8_t b = block[pos];
				pos++;
				literal_len += b;

				while (b == 255) {
					uint8_t b = block[pos];
					pos++;
					literal_len += b;
				}
			}

			std::vector<uint8_t> literals(begin(block) + pos, begin(block) + pos + literal_len);
			pos += literal_len;
			write_literals(data, literals);
			if (pos == block_len) break;


			//offset
			uint16_t offset = block[pos + 1] << 8 | block[pos];
			pos += 2;

			//match len
			long match_len = token & 0x0F;
			if (match_len == 15) {
				uint8_t b = block[pos];
				pos++;
				match_len += b;

				while (b == 255) {
					b = block[pos];
					pos++;
					match_len += b;
				}
			}
			match_len += 4;
			write_run(data, offset, match_len);
		}
	}

	os.write(reinterpret_cast<char*>(data.data()), data.size());
}
int main(int argc, char ** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}.", argv[1]);
		return 1;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::println("Error: fail opening input file {}.", argv[2]);
		return 1;
	}

	if (read_header(is) == 1) return 1;
	decode_blocks(is, os);
	return 0;
}
#include <cstdint>
#include <print>
#include <fstream>
#include <iostream>
#include <bit>

#include <string>
#include <vector>

template<typename T>
T read_number(std::ifstream& is, uint8_t nbytes) {
	T val = 0;
	is.read(reinterpret_cast<char*>(&val), nbytes);
	val = std::byteswap(val);

	val = val >> (8 * (sizeof(T) - nbytes));
	return val;
}
struct infoentry {
	uint32_t data_offset_;
	uint32_t id_;

	infoentry(uint32_t data_offset, uint32_t id): data_offset_(data_offset), id_(id){}
};

void copy(std::ifstream& is, std::vector<uint8_t>& decoded_data, int nbytes) {
	for (int i = 0; i < nbytes; i++) {
		decoded_data.push_back(is.get());
	}
}

void run(std::ifstream& is, std::vector<uint8_t>& decoded_data, uint8_t b1) {
	uint8_t b2 = is.get();
	uint16_t distance = 0;
	distance = (b1 & 0x3F) << 5;
	distance = distance | ((b2 >> 3) & 0x1F);

	uint8_t len = (b2 & 0x07) +3;

	size_t initial_pos = decoded_data.size() - distance;
	for (int c = 0; c < len; c++) {
		decoded_data.push_back(decoded_data[initial_pos + c]);
	}
}
//true if stop decoding current record
bool lz77_variant(std::ifstream& is, std::vector<uint8_t>& decoded_data) {
	uint8_t b = is.get();
	if (b == 0) return true;
	else if (b >= 1 and b <= 9) { 
		copy(is, decoded_data, b); 
	}
	else if (b >= 0x9 and b <= 0x7F) {
		decoded_data.push_back(b);
	}
	else if (b >= 0x80 and b <= 0xBF) {
		run(is, decoded_data, b);
	}
	else {
		decoded_data.push_back(0x20);
		uint8_t new_b = b & 0x7F;
		decoded_data.push_back(new_b);
	}

	return false;
}
bool decompress(std::ifstream& is, std::ofstream& os) {
	//step 1 ------------------------------------------
	//write BOM in output file
	os.put(static_cast<char>(0xEF));
	os.put(static_cast<char>(0xBB));
	os.put(static_cast<char>(0xBF));

	//read PDB header
	std::string db_name(32, ' ');
	is.read(db_name.data(), 32);

	is.ignore(4);

	uint32_t creation_date = read_number<uint32_t>(is, 4);
	
	is.ignore(5 * 4);

	std::string type(4, ' ');
	is.read(type.data(), 4);

	std::string creator(4, ' ');
	is.read(creator.data(), 4);

	is.ignore(2 * 4);

	uint16_t num_records = read_number<uint16_t>(is, 2);

	//print header values (stdout)
	std::print("PDB name: {}\nCreation date (s): {}\nType: {}\nCreator: {}\nRecords: {}\n", db_name, creation_date, type, creator, num_records);

	//step 2 ----------------------------------------
	std::vector<infoentry> infoentries;

	for (int c = 0; c < num_records; c++) {
		uint32_t data_offset = read_number<uint32_t>(is, 4);
		is.ignore(1);
		uint32_t id = read_number<uint32_t>(is, 3);
		
		//save infoentry
		infoentries.emplace_back(data_offset, id);
		//print infoentry (cout)
		std::print("{} - offset: {} - id: {}\n", c, data_offset, id);
	}
	std::print("\n");
	
	//step 3 ------------------------------------------------
	//analyze first record
	is.seekg(infoentries[0].data_offset_);
	uint16_t compression = read_number<uint16_t>(is, 2);
	is.ignore(2);
	uint32_t text_len = read_number<uint32_t>(is, 4);
	uint16_t record_count = read_number<uint16_t>(is, 2);
	uint16_t record_size = read_number<uint16_t>(is,  2);
	uint16_t encryption_type = read_number<uint16_t>(is, 2);
	
	std::print("Compression: {}\nTextLength: {}\nRecordCount: {}\nRecordSize: {}\nEncryptionType: {}\n\n", compression, text_len, record_count, record_size, encryption_type);
	//step 4 -------------------------------
	
	size_t tmp_text_len = 0;
	for (int c = 1; c <= record_count; c++) {
		//step to record position
		is.seekg(infoentries[c].data_offset_);
		std::vector<uint8_t> decoded_data;
		bool stop_decoding_record = false;
		while (decoded_data.size() < record_size and !stop_decoding_record and tmp_text_len+decoded_data.size()<text_len) {
			stop_decoding_record = lz77_variant(is, decoded_data);
		}

		text_len += decoded_data.size();
		os.write(reinterpret_cast<char*>(decoded_data.data()), decoded_data.size());
	}

	return true;
}


int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}", argc - 1);
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}", argv[1]);
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::println("Error: fail opening output file {}", argv[2]);
		return EXIT_FAILURE;
	}

	if (decompress(is, os)) return EXIT_SUCCESS;
	else return EXIT_FAILURE;
}
#include <cstdint>
#include <print>
#include <fstream>
#include <iostream>
#include <bit>

#include <string>
#include <vector>

struct infoentry {
	uint32_t data_offset_;
	uint32_t id_;

	infoentry(uint32_t data_offset, uint32_t id) : data_offset_(data_offset), id_(id) {}
};
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

	uint32_t creation_date;
	is.read(reinterpret_cast<char*>(&creation_date), 4);
	creation_date = std::byteswap(creation_date);

	is.ignore(5 * 4);

	std::string type(4, ' ');
	is.read(type.data(), 4);

	std::string creator(4, ' ');
	is.read(creator.data(), 4);

	is.ignore(2 * 4);

	uint16_t num_records;
	is.read(reinterpret_cast<char*>(&num_records), 2);
	num_records = std::byteswap(num_records);

	//print header values (stdout)
	std::print("PDB name: {}\nCreation date (s): {}\nType: {}\nCreator: {}\nRecords: {}\n", db_name, creation_date, type, creator, num_records);

	//step 2 ----------------------------------------
	std::vector<infoentry> infoentries;

	for (int c = 0; c < num_records; c++) {
		uint32_t data_offset = 0;
		is.read(reinterpret_cast<char*>(&data_offset), 4);
		data_offset = std::byteswap(data_offset);

		is.ignore(1);

		uint32_t id = 0;
		is.read(reinterpret_cast<char*>(&id), 3);
		id = std::byteswap(id) >> 8;

		//save infoentry
		infoentries.emplace_back(data_offset, id);
		//print infoentry (cout)
		std::print("{} - offset: {} - id: {}\n", c, data_offset, id);
	}

	//step 3 ------------------------------------------------

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
	decompress(is, os);

}
#include <cstdlib>
#include <bitset>
#include <print>
#include <iostream>
#include <fstream>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, 
	size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

int16_t mask(char n_ones) {
	std::bitset<16> mask;
	mask = 0;
	for (int i = 0; i < n_ones; i++) {
		mask = (mask << 1);
		mask |= 1;
	}

	return static_cast<int16_t>(mask.to_ulong());
}
struct buffer_11_writer {
	std::bitset<16> buffer_;
	char fill_index_;					// from 0 to 15, next bit to be inserted
	std::ostream& os_;

	//default constructor
	buffer_11_writer(std::ostream& outputStream)
		: buffer_(0), fill_index_(0), os_(outputStream) {
	}

	int insert_number(int16_t num) {
		//11 bit can't fit,
			//-write the number available (in little endian) and set the number as a local variable
			//-print buffer
			//-write remaining bits

		//check if buffer needs to be printed

		if (fill_index_ + 11 <= 16)
		{
			std::cout << "buffer_ pre writing-> " << buffer_ << std::endl;
			std::cout << "writing first 11 bits of " << std::bitset<16>(num) << " aka " << std::bitset<11>(num & 0x7ff) << std::endl;

			//11 bit can fit, write it in
			buffer_ |= (num & 0x7ff) << fill_index_;
			fill_index_ += 11;

			std::cout << "buffer_ post writing-> " << buffer_ << "\n------------------------------------"<< std::endl;

			
		}
		else
		{
			//11 bit can't fit,
			//-write the number available (in little endian) and set the number as a local variable	
			char free_bits = 16 - fill_index_;

			std::cout << "buffer_ pre writing-> " << buffer_ << std::endl;
			std::cout << "writing first "<< static_cast<int>(free_bits) <<" bits of " << std::bitset<16>(num) << " aka " << std::bitset<11>((num & mask(free_bits)))<< "(mask -> "<< std::bitset<11>(mask(free_bits))<<")" << std::endl;


			buffer_ |= (num & mask(free_bits)) << fill_index_;
			fill_index_ += free_bits;

			std::cout << "buffer_ post writing-> " << buffer_ << std::endl;

			if (fill_index_ != 16) {
				std::println(std::cerr, "Buffer was not filled properly, current offset: {}", fill_index_);
				return EXIT_FAILURE;
			}
			//-print buffer
			uint16_t val = static_cast<uint16_t>(buffer_.to_ulong());
			raw_write(os_, val);

			//write remaining bits
			buffer_ = 0;
			fill_index_ = 0;

			std::cout << "buffer_ pre writing-> " << buffer_ << std::endl;
			std::cout << "writing last " << static_cast<int>(11-free_bits) << " bits of " << std::bitset<16>(num) << " aka " << std::bitset<11>((num & mask(free_bits))) << "(mask -> " << std::bitset<11>(mask(11 - free_bits)) << ")" << std::endl;

			buffer_ |= (num >> free_bits) & mask(11 - free_bits);
			fill_index_ += (11 - free_bits);

			std::cout << "buffer_ post writing-> " << buffer_ << "\n------------------------------------" << std::endl;
		}

		//check if buffer needs to be printed
		if (fill_index_ == 16) {
			uint16_t val = static_cast<uint16_t>(buffer_.to_ulong());
			raw_write(os_, val);

			buffer_ = 0;
			fill_index_ = 0;
		}
	}
	void print() {
		if (fill_index_ != 0) {
			std::cout << "buffer_ to be printed-> " << buffer_ << std::endl;

			uint16_t val = static_cast<uint16_t>(buffer_.to_ulong());
			raw_write(os_, val);

			buffer_ = 0;
			fill_index_ = 0;
		}
	}

};

int main(int argc, char** argv) {
	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream inputFile(argv[1]/*, std::ios::binary*/);
	if (!inputFile) {
		return EXIT_FAILURE;
	}
	ofstream outputFile(argv[2], std::ios::binary);
	if (!outputFile) {
		return EXIT_FAILURE;
	}

	buffer_11_writer buff( outputFile);
	int16_t num;
	while (inputFile >> num) {
		buff.insert_number(num);
	}
	buff.print();
}
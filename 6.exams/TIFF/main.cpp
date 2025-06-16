#include <cstdlib>
#include <fstream>
#include <iostream>
#include <print>

#include <vector>
#include <string>

//contains .tif singificant infos
struct tif_header {
	uint32_t image_width = 0;
	uint32_t image_height = 0;
	uint16_t bitspersample = 0;
	uint16_t compression = 0;
	uint16_t photo_interpretation = 0;
	uint32_t rows_per_strip = 0;
	uint32_t strip_byte_counts = 0;
	//aka strip_offset
	uint32_t image_offset = 0;
};

struct image {
	std::vector<uint8_t> data_;
	uint32_t r_ = 0;
	uint32_t c_ = 0;
	
	image(uint32_t width, uint32_t height) : r_(height), c_(width) {
		data_.resize(r_ * c_);
	}
	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	size_t rawsize() {
		return r_ * c_;
	}
};

int tiff_to_pam(std::ifstream& is, std::ofstream& os) {
	//read header
	std::string byte_order(2, ' ');
	is.read(byte_order.data(), 2);
	if (byte_order != "II") {
		std::println("Error: byte order is {}, but should be 'II'.", byte_order);
		return EXIT_FAILURE;
	}

	uint16_t number;
	is.read(reinterpret_cast<char*>(&number), 2);
	if (number != 42) {
		std::println("Error: input file is not .tif, magic number is {} but should be 42.", number);
		return EXIT_FAILURE;
	}

	tif_header header;
	uint32_t ifd_offset;
	is.read(reinterpret_cast<char*>(&ifd_offset), 4);
	while (1) {
		//move to next IFD
		is.seekg(ifd_offset, is.beg);
		uint16_t n_dir_entries;
		is.read(reinterpret_cast<char*>(&n_dir_entries), 2);

		while (n_dir_entries-- > 0) {
			//read dir entry
			uint16_t tag;
			uint16_t type;
			uint32_t count;
			uint32_t value_offset;

			is.read(reinterpret_cast<char*>(&tag), 2);
			is.read(reinterpret_cast<char*>(&type), 2);
			is.read(reinterpret_cast<char*>(&count), 4);
			is.read(reinterpret_cast<char*>(&value_offset), 4);

			//analyze dir entry only if meaningfull
			if (tag == 256) {	//ImageWidth
				if (count != 1) {
					std::println("Error: ImageWidth count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				header.image_width = value_offset;
			}
			else if (tag == 257) {	//ImageLength
				if (count != 1) {
					std::println("Error: ImageLength count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				header.image_height = value_offset;
			}
			else if (tag == 258) {	//BitsPerSample
				if (count != 1) {
					std::println("Error: BitsPerSample count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				if (value_offset !=8) {
					std::println("Error: BitsPerSample should be 8 but are {}.", value_offset);
					return EXIT_FAILURE;
				}
				header.bitspersample = value_offset;
			}
			else if (tag == 259) {	//Compression
				if (count != 1) {
					std::println("Error: Compression count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				if (value_offset != 1) {
					std::println("Error: Compression should be 1 but is {}.", value_offset);
					return EXIT_FAILURE;
				}
				header.compression = value_offset;
			}
			else if (tag == 262) {	//PhotometricInterpretation
				if (count != 1) {
					std::println("Error: PhotometricInterpretation count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				if (value_offset != 1) {
					std::println("Error: PhotometricInterpretation should be 1 but is {}.", value_offset);
					return EXIT_FAILURE;
				}
				header.photo_interpretation = value_offset;
			}
			else if (tag == 278) {	//RowsPerStrip
				if (count != 1) {
					std::println("Error: RowsPerStrip count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				header.rows_per_strip = value_offset;
			}
			else if (tag == 279) {	//StripByteCounts
				if (count != 1) {
					std::println("Error: StripByteCounts count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				header.strip_byte_counts = value_offset;
			}
			else if (tag == 273) {	//StripOffsets
				if (count != 1) {
					std::println("Error: StripOffsets count should be 1 but is {}.", count);
					return EXIT_FAILURE;
				}
				header.image_offset = value_offset;
			}
		}
		is.read(reinterpret_cast<char*>(&ifd_offset), 4);
		if (ifd_offset == 0) break;	//if terminal ifd stop, otherwise move to next ifd and read it
	}

	//read image
	is.seekg(header.image_offset, is.beg);
	image img(header.image_width, header.image_height);
	is.read(img.rawdata(), img.rawsize());


	//write .pam image
	os << "P7\n";
	os << "WIDTH " << img.c_ << "\n";
	os << "HEIGHT " << img.r_ << "\n";
	os << "DEPTH " << 1 << "\n";
	os << "MAXVAL " << 255 << "\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";
	os.write(img.rawdata(), img.rawsize());
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}

	//eventually check extensione .tiff .pam
	std::ifstream is(argv[1], std::ios::binary);
	std::ofstream os(argv[2], std::ios::binary);
	if (!is or !os) {
		std::println("Error: fail opening input or output file.");
		return EXIT_FAILURE;
	}

	tiff_to_pam(is, os);
}
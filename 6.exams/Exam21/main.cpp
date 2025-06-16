#include <print>
#include <fstream>
#include <cstdio>

#include <vector>
#include <string>

struct image {
	std::vector<uint8_t> data_;
	uint32_t r_ = 0;
	uint32_t c_ = 0;

	image() {};
	image(uint32_t r, uint32_t c) {
		r_ = r;
		c_ = c;
		data_.resize(r * c);
	};

	void set_rows(uint32_t r) {
		r_ = r;
		data_.resize(r_ * c_);
	};
	void set_cols(uint32_t c) {
		c_ = c;
		data_.resize(r_ * c_);
	};

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	size_t rawsize() {
		return data_.size();
	}

	uint8_t& operator()(uint32_t r, uint32_t c) {
		return data_[r * c_ + c];
	}
};

int check_compression_inputs(std::string input_file, std::string output_file) {
	if (input_file.substr(input_file.size() - 4, input_file.size()) != ".pgm") {
		std::println("Error: input file should be .pgm");
		return EXIT_FAILURE;
	}
	if (output_file.substr(output_file.size() - 4, output_file.size()) != ".mlt") {
		std::println("Error: output file should be .mlt");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int check_decompression_inputs(std::string input_file, std::string output_file) {
	if (input_file.substr(input_file.size() - 4, input_file.size()) != ".mlt") {
		std::println("Error: input file should be .mlt");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int extract_pgm_image(std::ifstream& is, image& img) {
	//read header
	//magic number
	std::string header(2, ' ');
	is >> header;
	if (header != "P5") {
		std::println("Error: wrong .pgm header.");
		return EXIT_FAILURE;
	}

	//W
	std::string val(10, ' ');
	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy);
		is >> val;
	}
	img.set_cols(std::stoi(val));

	//H
	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy);
		is >> val;
	}
	img.set_rows(std::stoi(val));

	//maxval
	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy);
		is >> val;
	}
	if (std::stoi(val) != 255) {
		std::println("Error: .pgm maxval is {}, not 255.", std::stoi(val));
		return EXIT_FAILURE;
	}
	is.get();

	//pixel raster
	is.read(img.rawdata(), img.rawsize());
	return EXIT_SUCCESS;
}
void image_to_mlt(std::ofstream& os, image& img) {
	os << "MULTIRES";

	uint32_t c_ = img.c_;
	uint32_t r_ = img.r_;
	os.write(reinterpret_cast<char*>(&c_), 4);
	os.write(reinterpret_cast<char*>(&r_), 4);

	//determine pixels of the corresponding groups
	std::vector<uint8_t> l1;
	std::vector<uint8_t> l2;
	std::vector<uint8_t> l3;
	std::vector<uint8_t> l4;
	std::vector<uint8_t> l5;
	std::vector<uint8_t> l6;
	std::vector<uint8_t> l7;

	for (uint32_t r = 0; r < r_; r++) {
		for (uint32_t c = 0; c < c_; c++) {
			uint8_t val = img(r, c);

			//l1
			if ((r % 8 == 0) and (c % 8 == 0))	l1.push_back(val);
			//l2
			else if ((r % 8 == 0) and (c % 8 == 4))	l2.push_back(val);
			//l3
			else if ((r % 8 == 4) and (c % 4 == 0))	l3.push_back(val);
			//l4
			else if ((r % 4 == 0) and (c % 8 == 2)) l4.push_back(val);
			else if ((r % 4 == 0) and (c % 8 == 6)) l4.push_back(val);
			//l5
			else if ((r % 8 == 2) and (c % 2 == 0)) l5.push_back(val);
			else if ((r % 8 == 6) and (c % 2 == 0)) l5.push_back(val);
			//l6
			else if ((r % 2 == 0) and (c % 2 == 1))l6.push_back(val);
			//l7
			else if (r % 2 == 1) l7.push_back(val);
			else {
				std::println("No match found, aka error.");
			}
		}
	}

	os.write(reinterpret_cast<char*>(l1.data()), l1.size());
	os.write(reinterpret_cast<char*>(l2.data()), l2.size());
	os.write(reinterpret_cast<char*>(l3.data()), l3.size());
	os.write(reinterpret_cast<char*>(l4.data()), l4.size());
	os.write(reinterpret_cast<char*>(l5.data()), l5.size());
	os.write(reinterpret_cast<char*>(l6.data()), l6.size());
	os.write(reinterpret_cast<char*>(l7.data()), l7.size());
	return;
}
int mlt_to_levelpixels(std::ifstream& is, std::vector<std::vector<uint8_t>>& levels, uint32_t& r_, uint32_t& c_) {
	//read header
	std::string header(8, ' ');
	is.read(header.data(), 8);
	if (header != "MULTIRES") {
		std::println("Error: wrong input file header");
		return EXIT_FAILURE;
	}
	is.read(reinterpret_cast<char*>(&c_), 4);
	is.read(reinterpret_cast<char*>(&r_), 4);

	//calculate number of pixels per level
	std::vector<uint32_t> levels_size(7, 0);
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			//l1
			if ((r % 8 == 0) and (c % 8 == 0))	levels_size[0]++;
			//l2
			else if ((r % 8 == 0) and (c % 8 == 4))	levels_size[1]++;
			//l3
			else if ((r % 8 == 4) and (c % 4 == 0))	levels_size[2]++;
			//l4
			else if ((r % 4 == 0) and (c % 8 == 2))	levels_size[3]++;
			else if ((r % 4 == 0) and (c % 8 == 6)) levels_size[3]++;
			//l5
			else if ((r % 8 == 2) and (c % 2 == 0)) levels_size[4]++;
			else if ((r % 8 == 6) and (c % 2 == 0)) levels_size[4]++;
			//l6
			else if ((r % 2 == 0) and (c % 2 == 1)) levels_size[5]++;
			//l7
			else if (r % 2 == 1) levels_size[6]++;
		}
	}

	//read pixels per level
	for (int l = 0; l < 7; l++) {
		std::vector<uint8_t> level(levels_size[l]);
		is.read(reinterpret_cast<char*>(level.data()), levels_size[l]);
		levels.push_back(level);
	}
	return EXIT_SUCCESS;
}

void write_pgm_header(std::ofstream& os, uint32_t r_, uint32_t c_) {
	os << "P5 ";
	os << std::to_string(c_) << " ";
	os << std::to_string(r_) << " ";
	os << std::to_string(255) << "\n";
}
void levelpixels_to_l7(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_7.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	//reconstruct l7 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			//l1
			if ((r % 8 == 0) and (c % 8 == 0)) {
				os.write(reinterpret_cast<char*>(&levels[0][c_l1]), 1);
				c_l1++;
			}
			//l2
			else if ((r % 8 == 0) and (c % 8 == 4)) {
				os.write(reinterpret_cast<char*>(&levels[1][c_l2]), 1);
				c_l2++;
			}
			//l3
			else if ((r % 8 == 4) and (c % 4 == 0)) {
				os.write(reinterpret_cast<char*>(&levels[2][c_l3]), 1);
				c_l3++;
			}
			//l4
			else if ((r % 4 == 0) and (c % 8 == 2)) {
				os.write(reinterpret_cast<char*>(&levels[3][c_l4]), 1);
				c_l4++;
			}	
			else if ((r % 4 == 0) and (c % 8 == 6)) {
				os.write(reinterpret_cast<char*>(&levels[3][c_l4]), 1);
				c_l4++;
			}
			//l5
			else if ((r % 8 == 2) and (c % 2 == 0)) {
				os.write(reinterpret_cast<char*>(&levels[4][c_l5]), 1);
				c_l5++;
			}
			else if ((r % 8 == 6) and (c % 2 == 0)) {
				os.write(reinterpret_cast<char*>(&levels[4][c_l5]), 1);
				c_l5++;
			}
			//l6
			else if ((r % 2 == 0) and (c % 2 == 1)) {
				os.write(reinterpret_cast<char*>(&levels[5][c_l6]), 1);
				c_l6++;
			}
			//l7
			else if (r % 2 == 1) {
				os.write(reinterpret_cast<char*>(&levels[6][c_l7]), 1);
				c_l7++;
			}
		}
	}
}
void levelpixels_to_l6(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_6.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l6 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 2 == 1) {
				img(r, c) = img(r - 1, c);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
				//l2
				else if ((r % 8 == 0) and (c % 8 == 4)) {
					img(r, c) = levels[1][c_l2];
					c_l2++;
				}
				//l3
				else if ((r % 8 == 4) and (c % 4 == 0)) {
					img(r, c) = levels[2][c_l3];
					c_l3++;
				}
				//l4
				else if ((r % 4 == 0) and (c % 8 == 2)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
				else if ((r % 4 == 0) and (c % 8 == 6)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
				//l5
				else if ((r % 8 == 2) and (c % 2 == 0)) {
					img(r, c) = levels[4][c_l5];
					c_l5++;
				}
				else if ((r % 8 == 6) and (c % 2 == 0)) {
					img(r, c) = levels[4][c_l5];
					c_l5++;
				}
				//l6
				else if ((r % 2 == 0) and (c % 2 == 1)) {
					img(r, c) = levels[5][c_l6];
					c_l6++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}
void levelpixels_to_l5(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_5.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l5 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 2 == 1) {
				img(r, c) = img(r - 1, c);
			}
			else if (c % 2 == 1) {
				img(r, c) = img(r, c-1);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
				//l2
				else if ((r % 8 == 0) and (c % 8 == 4)) {
					img(r, c) = levels[1][c_l2];
					c_l2++;
				}
				//l3
				else if ((r % 8 == 4) and (c % 4 == 0)) {
					img(r, c) = levels[2][c_l3];
					c_l3++;
				}
				//l4
				else if ((r % 4 == 0) and (c % 8 == 2)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
				else if ((r % 4 == 0) and (c % 8 == 6)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
				//l5
				else if ((r % 8 == 2) and (c % 2 == 0)) {
					img(r, c) = levels[4][c_l5];
					c_l5++;
				}
				else if ((r % 8 == 6) and (c % 2 == 0)) {
					img(r, c) = levels[4][c_l5];
					c_l5++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}
void levelpixels_to_l4(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_4.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l4 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 4 != 0) {
				img(r, c) = img(r - 1, c);
			}
			else if (c % 2 == 1) {
				img(r, c) = img(r, c - 1);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
				//l2
				else if ((r % 8 == 0) and (c % 8 == 4)) {
					img(r, c) = levels[1][c_l2];
					c_l2++;
				}
				//l3
				else if ((r % 8 == 4) and (c % 4 == 0)) {
					img(r, c) = levels[2][c_l3];
					c_l3++;
				}
				//l4
				else if ((r % 4 == 0) and (c % 8 == 2)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
				else if ((r % 4 == 0) and (c % 8 == 6)) {
					img(r, c) = levels[3][c_l4];
					c_l4++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}
void levelpixels_to_l3(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_3.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l3 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 4 != 0) {
				img(r, c) = img(r - 1, c);
			}
			else if (c % 4 != 0) {
				img(r, c) = img(r, c - 1);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
				//l2
				else if ((r % 8 == 0) and (c % 8 == 4)) {
					img(r, c) = levels[1][c_l2];
					c_l2++;
				}
				//l3
				else if ((r % 8 == 4) and (c % 4 == 0)) {
					img(r, c) = levels[2][c_l3];
					c_l3++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}
void levelpixels_to_l2(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_2.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l2 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 8 != 0) {
				img(r, c) = img(r - 1, c);
			}
			else if (c % 4 != 0) {
				img(r, c) = img(r, c - 1);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
				//l2
				else if ((r % 8 == 0) and (c % 8 == 4)) {
					img(r, c) = levels[1][c_l2];
					c_l2++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}
void levelpixels_to_l1(std::string filename, std::vector<std::vector<uint8_t>>& levels, uint32_t r_, uint32_t c_) {
	std::ofstream os(filename + "_1.pgm", std::ios::binary);
	write_pgm_header(os, r_, c_);

	uint32_t c_l1 = 0;
	uint32_t c_l2 = 0;
	uint32_t c_l3 = 0;
	uint32_t c_l4 = 0;
	uint32_t c_l5 = 0;
	uint32_t c_l6 = 0;
	uint32_t c_l7 = 0;

	image img(r_, c_);
	//reconstruct l1 image
	for (uint32_t r = 0; r < r_; ++r) {
		for (uint32_t c = 0; c < c_; ++c) {
			if (r % 8 != 0) {
				img(r, c) = img(r - 1, c);
			}
			else if (c % 8 != 0) {
				img(r, c) = img(r, c - 1);
			}
			else {
				//l1
				if ((r % 8 == 0) and (c % 8 == 0)) {
					img(r, c) = levels[0][c_l1];
					c_l1++;
				}
			}
		}
	}
	os.write(img.rawdata(), img.rawsize());
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::println("Error: program accepts 3 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}

	std::string mode(argv[1]);
	std::string input_file(argv[2]);
	std::string output_file(argv[3]);
	std::ifstream is(input_file, std::ios::binary);
	if (!is) {
		std::println("Error: fial opening input file {}.", input_file);
		return EXIT_FAILURE;
	}

	if (mode == "c") {
		std::ofstream os(output_file, std::ios::binary);
		if (check_compression_inputs(input_file, output_file) == 1) return EXIT_FAILURE;
		//extract image
		image img;
		if (extract_pgm_image(is, img) == 1) return EXIT_FAILURE;

		//write .mlt format
		image_to_mlt(os, img);
		return EXIT_SUCCESS;
	}
	if (mode == "d") {
		if (check_decompression_inputs(input_file, output_file) == 1) return EXIT_FAILURE;
		//extract level pixels
		std::vector<std::vector<uint8_t>> levels;
		uint32_t rows, cols;
		mlt_to_levelpixels(is, levels, rows, cols);
		//reconstruct levels
		levelpixels_to_l7(output_file, levels, rows, cols);
		levelpixels_to_l6(output_file, levels, rows, cols);
		levelpixels_to_l5(output_file, levels, rows, cols);
		levelpixels_to_l4(output_file, levels, rows, cols);
		levelpixels_to_l3(output_file, levels, rows, cols);
		levelpixels_to_l2(output_file, levels, rows, cols);
		levelpixels_to_l1(output_file, levels, rows, cols);

		return EXIT_SUCCESS;
	}
	else {
		std::println("Error: first parameter should be 'c' or 'd', not {}.", mode);
		return EXIT_FAILURE;
	}
}


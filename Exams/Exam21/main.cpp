#include <cstdio>
#include <print>
#include <iostream>
#include <fstream>
#include <expected>

#include <string>
#include <vector>
#include <cmath>

template<typename T>
struct image {
	int h_ = 0;
	int w_ = 0;
	std::vector<T> data_;

	image(int h, int w) : h_(h), w_(w) {
		data_.resize(h * w);
	};

	T& operator()(int r, int c) {
		return data_[r * w_ + c];
	}
	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	size_t rawsize() {
		return h_ * w_ * sizeof(T);
	}
	
};

std::expected<image<uint8_t>, std::string>
extract_PGM_file(std::ifstream& is) {
	//read .pgm file
	std::string header;
	is >> header;
	if (header != "P5") {
		return std::unexpected("input file is not .pgm.");
	}

	int w, h, maxval;
	std::string val;
	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy, '\n');
		is >> val;
	}
	w = std::stoi(val);

	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy, '\n');
		is >> val;
	}
	h = std::stoi(val);

	is >> val;
	while (val[0] == '#') {
		std::string dummy;
		std::getline(is, dummy, '\n');
		is >> val;
	}
	maxval = std::stoi(val);
	is.get();

	image<uint8_t> img(h, w);
	is.read(img.rawdata(), img.rawsize());

	return img;
}
int compress_to_mlt(std::ifstream& is, std::ofstream& os) {
	//read .pgm file
	auto res = extract_PGM_file(is);
	if (!res) {
		std::print("Error: {}", res.error());
		return EXIT_FAILURE;
	}
	image<uint8_t> img = res.value();

	//write .mlt header
	os << "MULTIRES";
	uint32_t w = img.w_;
	uint32_t h = img.h_;
	os.write(reinterpret_cast<char*>(&w), 4);
	os.write(reinterpret_cast<char*>(&h), 4);

	//select level 1 pixels
	for (int r = 0; r < img.h_; r+=8) {
		for (int c = 0; c < img.w_; c+=8) {
			os.put(img(r, c));
		}
	}

	//select level 2 pixels
	for (int r = 0; r < img.h_; r += 8) {
		for (int c = 4; c < img.w_; c += 8) {
			os.put(img(r, c));
		}
	}

	//select level 3 pixels
	for (int r = 4; r < img.h_; r += 8) {
		for (int c = 0; c < img.w_; c += 4) {
			os.put(img(r, c));
		}
	}

	//select level 4 pixels
	for (int r = 0; r < img.h_; r += 4) {
		for (int c = 2; c < img.w_; c += 4) {
			os.put(img(r, c));
		}
	}

	//select level 5 pixels
	for (int r = 2; r < img.h_; r += 4) {
		for (int c = 0; c < img.w_; c += 2) {
			os.put(img(r, c));
		}
	}

	//select level 6 pixels
	for (int r = 0; r < img.h_; r += 2) {
		for (int c = 1; c < img.w_; c += 2) {
			os.put(img(r, c));
		}
	}

	//select level 7 pixels
	for (int r = 1; r < img.h_; r += 2) {
		for (int c = 0; c < img.w_; c += 1) {
			os.put(img(r, c));
		}
	}
	return EXIT_SUCCESS;
}

void write_PGM_header(int w, int h, std::ofstream& os) {
	os << "P5 ";
	os << w << " ";
	os << h << " ";
	os << 255 << "\n";
}
int decompress_from_mlt(std::ifstream& is, std::string prefix) {
	//read header
	std::string header(8, ' ');
	is.read(&header[0], 8);
	if (header != "MULTIRES") {
		std::print("Error: input file is not .mlt\n");
		return EXIT_FAILURE;
	}

	int w = 0;
	int h = 0;
	is.read(reinterpret_cast<char*>(&w), 4);
	is.read(reinterpret_cast<char*>(&h), 4);
	//reconstruct level 1-------------------------------------------
	std::ofstream f_l1(prefix + "_1.pgm", std::ios::binary);
	if (!f_l1) {
		std::println("Error: fail opnening file {}", prefix + "_1.pgm");
		return EXIT_FAILURE;
	}
	write_PGM_header(w, h, f_l1);

	int nblocks_columns = static_cast<int>(std::ceil(w / 8.));
	int nblocks_lines = static_cast<int>(std::ceil(h / 8.));
	int npixels_l1 = nblocks_columns * nblocks_lines;

	std::vector<uint8_t> pixels(npixels_l1);
	is.read(reinterpret_cast<char*>(pixels.data()), npixels_l1);

	for (int r = 0; r < h; r ++) {
		for (int c = 0; c < w; c ++) {
			auto pixel = pixels[(r / 8) * nblocks_columns + (c / 8)];
			f_l1.put(pixel);
		}
	}

	//reconstruct level 2-------------------------------------------
	std::ofstream f_l2(prefix + "_2.pgm", std::ios::binary);
	if (!f_l2) {
		std::println("Error: fail opnening file {}", prefix + "_2.pgm");
		return EXIT_FAILURE;
	}
	write_PGM_header(w, h, f_l2);
	nblocks_columns = static_cast<int>(std::ceil(w / 4.));
	nblocks_lines = static_cast<int>(std::ceil(h / 8.));
	int npixels_l2 = static_cast<int>(std::ceil(w / 4.))

	pixels.resize(npixels_l2);
	is.read(reinterpret_cast<char*>(pixels.data()), npixels_l2);

	for (int r = 0; r < h; r++) {
		for (int c = 0; c < w; c++) {
			auto pixel = pixels[(r / 8) * nblocks_columns + (c / 4)];
			f_l2.put(pixel);
		}
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::print("Error: program accepts 3 parameters, {} were provided.\n", argc - 1);
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		std::print("Error: fail to open input file {}", argv[2]);
		return EXIT_FAILURE;
	}

	if (std::string(argv[1]) == "c") {
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			std::print("Error: fail to open output file {}", argv[3]);
			return EXIT_FAILURE;
		}

		return compress_to_mlt(is, os);
	}
	if (std::string(argv[1]) == "d") {
		return decompress_from_mlt(is, std::string(argv[3]));
	}
	return EXIT_FAILURE;
}
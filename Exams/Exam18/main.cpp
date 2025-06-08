#include <cstdio>
#include <print>
#include <fstream>
#include <bit>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <memory>

#include <string>
#include <vector>
#include <array>

using RGB_float = std::array<float, 3>;
using RGB = std::array<uint8_t, 3>;

RGB float_to_char(float min, float max, RGB_float val) {
	RGB ret;
	float tmp = std::round(255 * std::pow((val[0] - min) / (max - min), 0.45));
	ret[0] = static_cast<uint8_t>(255 * std::pow((val[0] - min) / (max - min), 0.45));
	ret[1] = static_cast<uint8_t>(255 * std::pow((val[1] - min) / (max - min), 0.45));
	ret[2] = static_cast<uint8_t>(255 * std::pow((val[2] - min) / (max - min), 0.45));

	return ret;
}
template<typename T>
struct image {
	size_t r_ = 0;
	size_t c_ = 0;
	float min_ = 0;
	float max_ = 0;
	std::vector<T> data_;
	
	image() {};
	image(size_t r, size_t c) : r_(r), c_(c) {};
	size_t cols() const { return c_; }
	size_t rows() const { return r_; }

	void set_rows_cols(size_t r, size_t c) {
		r_ = r;
		c_ = c;
	};
	void find_minmax() {
		std::vector<float> v;
		for (int i = 0; i < data_.size(); i++) {
			v.push_back(data_[i][0]);
			v.push_back(data_[i][1]);
			v.push_back(data_[i][2]);
		}

		auto res = std::minmax_element(begin(v), end(v));
		min_ = static_cast<float>(*res.first);
		max_ = static_cast<float>(*res.second);
	}

	RGB operator() (int r, int c) {
		T val = data_[r * c_ + c];
		return float_to_char(min_, max_, val);
	}
};

bool check_scanline_start(std::ifstream& is, const image<RGB_float>& img) {
	uint32_t scanline_start = 0;
	is.read(reinterpret_cast<char*>(&scanline_start), 4);
	uint16_t ncols = static_cast<uint16_t>(img.cols());

	if ((scanline_start & 0xFF) != 2) return false;
	if ((scanline_start >> 8 & 0xFF) != 2) return false;
	if ((scanline_start >> 24 & 0xFF) != (ncols & 0xFF)) return false;
	if ((scanline_start >> 16 & 0xFF) != (ncols >> 8 & 0xFF)) return false;

	return true;
}
void read_RLE(std::ifstream& is, std::vector<uint8_t>& v) {
	//read L
	uint8_t L = is.get();
	if (L <= 127) {
		while (L-- > 0) {
			v.push_back(is.get());
		}
	}
	else if (L > 127) {
		uint8_t R = L - 128;
		uint8_t val = is.get();
		while (R-- > 0) {
			v.push_back(val);
		}
	}
}
void RGBE_to_RGBfloat_line(const std::vector<uint8_t>& v, image<RGB_float>& img) {
	int ncols = static_cast<int>(img.cols());

	for (int i = 0; i < ncols; i++) {
		uint8_t R = v[i];
		uint8_t G = v[i + ncols];
		uint8_t B = v[i + 2* ncols];
		uint8_t E = v[i + 3* ncols];

		float Rf = static_cast<float>(((R + 0.5) / 256.) * std::pow(2, E - 128));
		float Gf = static_cast<float>(((G + 0.5) / 256.) * std::pow(2, E - 128));
		float Bf = static_cast<float>(((B + 0.5) / 256.) * std::pow(2, E - 128));
		img.data_.push_back({ Rf, Gf, Bf });
	}
}

void write_PAM_file(std::ofstream& os, image<RGB_float>& img) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n", img.cols(), img.rows());
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			RGB val = img(r, c);
			os.write(reinterpret_cast<char*>(&val), 3);
		}
	}
}
int decode_hdr(std::ifstream& is, image<RGB_float>& img) {
	std::string line(100, ' ');
	std::getline(is, line);
	//read header
	if (line != "#?RADIANCE") {
		std::println("Error: first hdr line should be '#?RADIANCE'.");
		return EXIT_FAILURE;
	}
	while (1) {
		std::getline(is, line);
		if (line.size() == 0) break;
		if (line.substr(0, 6) == "FORMAT" and line != "FORMAT=32-bit_rle_rgbe") {
			std::println("Error: forma shoudl be '2-bit_rle_rgbe'.");
			return EXIT_FAILURE;
		}
	}

	//resolution string
	std::getline(is, line);
	std::string rows = line.substr(3, line.find(" +X ")-3);
	std::string cols = line.substr(line.find(" +X ")+4, line.size());
	img.set_rows_cols(std::stoi(rows), std::stoi(cols));

	size_t nrows = img.rows();
	while (nrows--> 0) {
		//read one line per time
		if (!check_scanline_start(is, img)) {
			std::println("Error: wrong scanline start.");
			return EXIT_FAILURE;
		}

		//read cols * 4 values with RLE algorithm
		std::vector<uint8_t> RGBE_line;
		while (RGBE_line.size() < 4 * img.cols()) {
			read_RLE(is, RGBE_line);
		}
		RGBE_to_RGBfloat_line(RGBE_line, img);
	}

	return EXIT_SUCCESS;
}


int main(int argc, char** argv) {
	//parameters
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}
	
	//in out files
	std::string ifile(argv[1]);
	if (ifile.substr(ifile.size() - 4, ifile.size()) != ".hdr") {
		std::println("Error: wrong input format {}, should be .hdr.", ifile);
		return EXIT_FAILURE;
	}
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::println("Error: fail openin ginput file {}.", ifile);
		return EXIT_FAILURE;
	}
	std::string ofile(argv[2]);
	if (ofile.substr(ofile.size() - 4, ofile.size()) != ".pam") {
		ofile.append(".pam");
	}
	std::ofstream os(ofile, std::ios::binary);

	//decode .hdr
	image<RGB_float> img;
	if (decode_hdr(is, img) == 1) return EXIT_FAILURE;
	//find min max(for conversion to rgb)
	img.find_minmax();
	//pritn in .pam format
	write_PAM_file(os, img);

	return EXIT_SUCCESS;
}
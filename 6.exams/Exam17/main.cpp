#include <cstdlib>
#include <print>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <string>
#include <vector>
#include <bit>
#include <array>

using RGB = std::array<uint8_t, 3>;

template<typename T>
struct image {
	size_t c_ = 0;
	size_t r_ = 0;
	std::vector<T> data_;

	image() {};
	image(size_t rows, size_t columns) : r_(rows), c_(columns) {};

	void set_rows_cols(size_t rows, size_t cols) {
		r_ = rows;
		c_ = cols;
	}
	size_t size() { return c_ * r_; }
	size_t rows() { return r_; }
	size_t cols() { return c_; }
	T& at(size_t r, size_t c) {
		return data_[r * c_ + c];
	}
	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	size_t rawsize() { return c_ * r_ * sizeof(T); }
};

bool read_pgm(std::ifstream& is, image<uint16_t>& img) {
	std::string header(2, ' ');
	is.read(reinterpret_cast<char*>(header.data()), 2);
	if (header != "P5") {
		std::println("Error: worng .pgm header.");
		return false;
	}
	is.get();

	size_t columns = 0;
	is >> columns;

	size_t rows =0;
	is >> rows;

	size_t maxval = 0;
	is >> maxval;
	is.get();

	img.set_rows_cols(rows, columns);
	for (size_t i = 0; i < img.size(); i++) {
		uint16_t val = 0;
		is.read(reinterpret_cast<char*>(&val), 2);
		img.data_.push_back(std::byteswap(val));
	}

	return true;
}
void save_pgm_8bpp(std::ofstream& os, image<uint8_t>& img) {
	std::print(os, "P5\n{} {}\n255\n", img.cols(), img.rows());
	os.write(img.rawdata(), img.rawsize());
}
void save_ppm_24bpp(std::ofstream& os, image<RGB>& img) {
	std::print(os, "P6\n{} {}\n255\n", img.cols(), img.rows());
	os.write(img.rawdata(), img.rawsize());
}
bool is_green(int r, int c) {
	if ((r % 2 == 0 and c % 2 == 1) or (r % 2 == 1 and c % 2 == 0)) return true;
	else return false;
}
bool is_red(int r, int c) {
	if (r % 2 == 0 and c % 2 == 0) return true;
	else return false;
}
bool is_blue(int r, int c) {
	if (r % 2 == 1 and c % 2 == 1) return true;
	else return false;
}
void interpolate_green(image<RGB>& img) {
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			if (is_green(r,c)) continue; //green, not interpolate now
			RGB& pixel = img.at(r, c);
			uint64_t channel = is_red(r,c) ? 0 : 2; //red, else blue

			RGB x1 = (r - 2 >= 0) ?			img.at(r - 2, c) : RGB{0,0,0};
			RGB x3 = (c - 2 >= 0) ?			img.at(r, c - 2) : RGB{ 0, 0, 0 };
			RGB x5 =						img.at(r, c);
			RGB x7 = (c + 2 < img.cols()) ? img.at(r, c + 2) : RGB{ 0, 0, 0 };
			RGB x9 = (r + 2 < img.rows()) ? img.at(r + 2, c) : RGB{ 0, 0, 0 };

			RGB g2 = (r - 1 >= 0) ?			img.at(r - 1, c) : RGB{ 0, 0, 0 };
			RGB g4 = (c-1>=0)?				img.at(r, c - 1): RGB{ 0, 0, 0 };
			RGB g6 = (c+1<img.cols())?		img.at(r, c + 1): RGB{ 0, 0, 0 };
			RGB g8 = (r+1<img.rows())?		img.at(r + 1, c): RGB{ 0, 0, 0 };

			size_t dH = std::abs(g4[1] - g6[1]) + std::abs(x5[channel] - x3[channel] + x5[channel] - x7[channel]);
			size_t dV = std::abs(g2[1] - g8[1]) + std::abs(x5[channel] - x1[channel] + x5[channel] - x9[channel]);
			
			int val;
			if (dH < dV)		val = ((g4[1] + g6[1]) / 2) + ((x5[channel] - x3[channel] + x5[channel] - x7[channel]) / 4);
			else if (dH > dV)	val = ((g2[1] + g8[1]) / 2) + ((x5[channel] - x1[channel] + x5[channel] - x9[channel]) / 4);
			else				val = ((g2[1] + g4[1] + g6[1] + g8[1]) / 4) + ((x5[channel] - x1[channel] + x5[channel] - x3[channel] + x5[channel] - x7[channel] + x5[channel] - x9[channel]) / 8);
			pixel[1] = std::clamp(val, 0, 255);
		}
	}
}
void interpolate_rb(image<RGB>& img) {
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			RGB& pixel = img.at(r, c);
			//green
			if (is_green(r, c)) {
				if (r % 2 == 1 and c % 2 == 0) {
					//red	-> top bottom
					//blue	-> left right
					int r1 = (r - 1 >= 0) ? img.at(r - 1, c)[0] : 0;
					int r2 = (r + 1 < img.rows()) ? img.at(r + 1, c)[0] : 0;
					int b1 = (c - 1 >= 0) ? img.at(r, c - 1)[2] : 0;
					int b2 = (c + 1 < img.cols()) ? img.at(r, c + 1)[2] : 0;

					pixel[0] = std::clamp((r1 + r2) / 2, 0, 255);
					pixel[2] = std::clamp((b1 + b2) / 2, 0, 255);
				}
				else if (r % 2 == 0 and c % 2 == 1) {
					//blue	-> top bottom
					//red	-> left right
					int b1 = (r - 1 >= 0) ? img.at(r - 1, c)[2] : 0;
					int b2 = (r + 1 < img.rows()) ? img.at(r + 1, c)[2] : 0;
					int r1 = (c - 1 >= 0) ? img.at(r, c - 1)[0] : 0;
					int r2 = (c + 1 < img.cols()) ? img.at(r, c + 1)[0] : 0;

					pixel[0] = std::clamp((r1 + r2) / 2, 0, 255);
					pixel[2] = std::clamp((b1 + b2) / 2, 0, 255);

				}
			}
			//red or blue
			else {
				uint64_t channel = is_red(r,c) ? 2 : 0; //if red->2, if blue->0
				int g1 = (r-1 >= 0 and c-1 >=0)?				img.at(r-1, c-1)[1]: 0;
				int g3 = (r-1 >= 0 and c+1 <img.cols())?		img.at(r - 1, c+1)[1]: 0;
				int g5 = img.at(r,c)[1];
				int g7 = (r + 1 < img.rows() and c-1 >= 0)?		img.at(r + 1, c - 1)[1] : 0;
				int g9 = (r+1<img.rows() and c+1 < img.cols())? img.at(r + 1, c + 1)[1]: 0;

				int x1 = (r-1 >= 0 and c-1 >= 0)?				img.at(r - 1, c - 1)[channel]: 0;
				int x3 = (r-1 >= 0 and c+1 < img.cols())?		img.at(r - 1, c + 1)[channel]: 0;
				int x7 = (r+1 < img.rows() and c-1 >=0)?		img.at(r+1, c-1)[channel]: 0;
				int x9 = (r+1 < img.rows() and c+1 <img.cols())?img.at(r +1, c + 1)[channel]: 0;

				size_t dN = std::abs(x1 - x9) + std::abs(g5 - g1 + g5 - g9);
				size_t dP = std::abs(x3 - x7) + std::abs(g5 - g3 + g5 - g7);

				int val;
				if (dN < dP)	val = ((x1 + x9) / 2) + ((g5 - g1 + g5 - g9) / 4);
				else if (dN > dP)	val = ((x3 + x7) / 2) + ((g5 - g3 + g5 - g7) / 4);
				else			val = ((x1 + x3 + x7 + x9) / 4) + ((g5 - g1 + g5 - g3 + g5 - g7 + g5 - g9) /8);
				pixel[channel] = std::clamp(val, 0, 255);
			}
		}
	}
}


int main(int argc, char** argv) {
	//check parameters
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}
	std::string ifile(argv[1]);
	if(ifile.substr(ifile.size() - 4) != ".pgm") {
		std::println("Error: wrong input file, shoul dbe .pgm");
		return EXIT_FAILURE;
	}
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}.", ifile);
		return EXIT_FAILURE;
	}

	std::string ofile_prefix(argv[2]);
	//step 1---------------------------------------------------------------
	//extract from pgm
	image<uint16_t> img;
	if (!read_pgm(is, img)) return EXIT_FAILURE;

	//converto to grayscale
	image<uint8_t> img_grayscale;
	img_grayscale.set_rows_cols(img.rows(), img.cols());
	for (const auto pixel : img.data_) {
		int gray_val = static_cast<int>(std::round(pixel / 256.));
		gray_val = std::clamp(gray_val, 0, 255);
		img_grayscale.data_.push_back(gray_val);
	}

	//save as pgm
	std::ofstream os_grayscale(ofile_prefix + "_gray.pgm", std::ios::binary);
	save_pgm_8bpp(os_grayscale, img_grayscale);
	
	//step 2---------------------------------------------------------
	//create rgb image
	image<RGB> img_rgb;
	img_rgb.set_rows_cols(img.rows(), img.cols());
	for (size_t r = 0; r < img_rgb.rows(); r++) {
		for (size_t c = 0; c < img_rgb.cols(); c++) {
			RGB pixel{0,0,0};

			if (r % 2 == 0 and c % 2 == 0)		pixel[0] = img_grayscale.at(r, c);
			else if (r % 2 == 1 and c % 2 == 1) pixel[2] = img_grayscale.at(r, c);
			else								pixel[1] = img_grayscale.at(r, c);
			img_rgb.data_.push_back(pixel);
		}
	}
	//save rgb image as .ppm
	std::ofstream os_rgb(ofile_prefix + "_bayer.ppm", std::ios::binary);
	save_ppm_24bpp(os_rgb, img_rgb);
		
	//step 3---------------------------------------------------------
	interpolate_green(img_rgb);
	std::ofstream os_rgb_green(ofile_prefix + "_green.ppm", std::ios::binary);
	save_ppm_24bpp(os_rgb_green, img_rgb);

	interpolate_rb(img_rgb);
	std::ofstream os_rgb_interp(ofile_prefix + "_interp.ppm", std::ios::binary);
	save_ppm_24bpp(os_rgb_interp, img_rgb);

	return EXIT_SUCCESS;
}
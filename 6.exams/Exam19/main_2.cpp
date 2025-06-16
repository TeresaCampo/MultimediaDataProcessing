#include <print>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <expected>
#include <cmath>

#include <vector>
#include <string>
#include <array>
#include <map>

template<typename T>
struct image {
	std::vector<T> data_;
	uint32_t h_ = 0;
	uint32_t w_ = 0;

	image(uint32_t h, uint32_t w) : h_(h), w_(w) {
		data_.resize(h_ * w_);
	}
	uint32_t height() {
		return h_;
	}
	uint32_t width() {
		return w_;
	}
	size_t num_pixels() {
		return h_ * w_;
	}

	T& operator()(uint32_t r, uint32_t c) {
		return data_[r * w_ + c];
	}

	size_t rawsize() {
		return data_.size() * sizeof(T);
	}
	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};

std::expected<image < std::array<int8_t, 3>>, int>
read_BMP_image(std::ifstream& is) {
	//read file header
	std::string header;
	is.read(header.data(), 2);

	uint32_t file_size;
	is.read(reinterpret_cast<char*>(&file_size), 4);

	uint32_t dummy;
	is.read(reinterpret_cast<char*>(&dummy), 2);
	is.read(reinterpret_cast<char*>(&dummy), 2);

	uint32_t image_offset;
	is.read(reinterpret_cast<char*>(&image_offset), 4);

	//read info header
	uint32_t header_size;
	is.read(reinterpret_cast<char*>(&header_size), 4);

	int32_t w;
	int32_t h;
	int16_t cp;
	int16_t d;

	is.read(reinterpret_cast<char*>(&w), 4);
	is.read(reinterpret_cast<char*>(&h), 4);
	is.read(reinterpret_cast<char*>(&cp), 2);
	if (cp != 1) {
		std::println("Error: image has {} color planes, but should have 1.", cp);
		return std::unexpected(EXIT_FAILURE);

	}
	is.read(reinterpret_cast<char*>(&d), 2);
	if (d != 8) {
		std::println("Error: image has {} bpp, but should have 8.", d);
		return std::unexpected(EXIT_FAILURE);

	}

	uint32_t compression_method;
	is.read(reinterpret_cast<char*>(&compression_method), 4);
	if (compression_method != 0) {
		std::println("Error: image doesn't use BI_RGB comrpession method.");
		return std::unexpected(EXIT_FAILURE);
	}
	
	is.read(reinterpret_cast<char*>(&dummy), 4);
	is.read(reinterpret_cast<char*>(&dummy), 4);
	is.read(reinterpret_cast<char*>(&dummy), 4);

	size_t num_colors = 0;
	is.read(reinterpret_cast<char*>(&num_colors), 4);
	if (num_colors == 0) {
		num_colors = static_cast<size_t>(std::pow(2, 8));
	}
	is.read(reinterpret_cast<char*>(&dummy), 4);
	
	//color table
	auto pos = is.tellg();
	std::map<uint8_t, std::array<uint8_t, 3>> color_map;
	for(size_t i = 0; i<num_colors; i++){
		uint8_t B = is.get();
		uint8_t G = is.get();
		uint8_t R = is.get();
		uint8_t zerp = is.get();
		color_map[i] = { R,G,B };
	}

	//image
	is.seekg(image_offset, std::ios::beg);
	uint32_t byte_raw_padding = 4 - (w) % 4;
	char padding[4];
	image < std::array<int8_t, 3>> img(h, w);

	std::vector<int8_t> row_pixel(w);
	std::vector<int8_t> row_padding_pixel(byte_raw_padding);

	for (int r = img.height() - 1; r >= 0; r--) {
		is.read(reinterpret_cast<char*>(row_pixel.data()), w);
		is.read(reinterpret_cast<char*>(row_padding_pixel.data()), byte_raw_padding);
		int row_pixel_pos = 0;
		for (uint32_t c = 0; c < img.width(); c++) {
			uint8_t color_index = row_pixel[row_pixel_pos];
			auto colors = color_map[color_index];
			img(r, c)[0] = color_map[color_index][0];
			img(r, c)[1] = color_map[color_index][1];
			img(r, c)[2] = color_map[color_index][2];
			row_pixel_pos ++;
		}
	}
	return img;
}

void write_PAM_image(std::ofstream& os, image < std::array<int8_t, 3>>& img) {
	os << "P7\n";
	os << "WIDTH " << img.width() << "\n";
	os << "HEIGHT " << img.height() << "\n";
	os << "DEPTH " << 3 << "\n";
	os << "MAXVAL " << 255 << "\n";
	os << "TUPLTYPE RGB\n";
	os << "ENDHDR\n";

	os.write(img.rawdata(), img.rawsize());
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accpets 2 pamaters.");
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening file {}", argv[1]);
		return EXIT_FAILURE;
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::println("Error: fail opening file {}", argv[2]);
		return EXIT_FAILURE;
	}

	auto res = read_BMP_image(is);
	if (!res) {
		return res.error();
	}
	image < std::array<int8_t, 3>> img = res.value();
	write_PAM_image(os, img);
}
#include <print>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <expected>

#include <vector>
#include <string>
#include <array>

struct bitreader {
	std::ifstream& is_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};
	uint32_t read_one_bit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 8;
		}

		n_--;
		return buffer_ >> n_ & 1;
	}

	uint32_t read_n_bits(int n_bits) {
		uint32_t val = 0;
		while (n_bits-- > 0) {
			val = val << 1 | read_one_bit();
		}
		return val;
	}
};
template<typename T>
struct image {
	std::vector<T> data_;
	uint32_t h_ = 0;
	uint32_t w_ = 0;

	image(uint32_t h, uint32_t w): h_(h), w_(w) {
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
	std::string header(2, ' ');
	is.read(header.data(), 2);
	if (header != "BM") {
		//std::println("Error: wrong header.");
		return std::unexpected(EXIT_FAILURE);
	}


	uint32_t file_size;
	is.read(reinterpret_cast<char*>(&file_size), 4);

	uint16_t dummy;
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
		//std::println("Error: image has {} color planes, but should have 1.", cp);
		return std::unexpected(EXIT_FAILURE);

	}
	is.read(reinterpret_cast<char*>(&d), 2);
	if (d != 24) {
		//std::println("Error: image has {} bpp, but should have 24.", d);
		return std::unexpected(EXIT_FAILURE);

	}

	uint32_t compression_method;
	is.read(reinterpret_cast<char*>(&compression_method), 4);
	if (compression_method != 0) {
		//std::println("Error: image doesn't use BI_RGB comrpession method.");
		return std::unexpected(EXIT_FAILURE);
	}

	//no color table
	//image
	is.seekg(image_offset, std::ios::beg);
	uint32_t bit_raw_padding = 32 - (w*24) % 32;
	image < std::array<int8_t, 3>> img(h, w);

	bitreader br(is);
	for (int r = img.height() - 1; r >= 0; r--) {
		for (uint32_t c = 0; c < img.width(); c++) {
			img(r, c)[2] = br.read_n_bits(8);
			img(r, c)[1] = br.read_n_bits(8);
			img(r, c)[0] = br.read_n_bits(8);
		}
		br.read_n_bits(bit_raw_padding);
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
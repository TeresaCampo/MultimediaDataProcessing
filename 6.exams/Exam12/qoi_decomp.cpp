#include <print>
#include <fstream>
#include <string>

#include <vector>
#include <array>
struct bitreader {
	std::ifstream& is_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	bitreader(std::ifstream& is) : is_(is) {};
	~bitreader() {};

	uint32_t read_one_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}

		n_--;
		return buffer_ >> n_ & 1;
	}

	uint32_t read_n_bits(uint32_t nbits) {
		uint32_t val = 0;
		while (nbits-- > 0) {
			val = val << 1 | read_one_bit();
		}
		return val;
	}
};

std::array<uint8_t, 4> last_pixel = { 0,0,0,255 };
std::array< std::array<uint8_t, 4>, 64> seen = {};

//shuld be an array of 4 uint8_t
template<typename T>
struct image {
	uint32_t w_ = 0;
	uint32_t h_ = 0;
	uint32_t c_ = 0;
	std::vector<T> img_;

	image(uint32_t w, uint32_t h, uint32_t c): w_(w), h_(h), c_(c){
		//img_.resize(w * h);
	}
	uint32_t width() { return w_; }
	uint32_t height() { return h_; }
	uint32_t channels() { return c_; }
	size_t size() { return w_ * h_; }

	const char* rawdata() { return reinterpret_cast<char*>(img_.data()); }
	size_t rawsize() { return size() * sizeof(T); };
};

void update_seen() {
	uint32_t index_position = (last_pixel[0] * 3 + last_pixel[1] * 5 + last_pixel[2] * 7 + last_pixel[3] * 11) % 64;
	seen[index_position] = last_pixel;
}
void qoi_op_RGB(bitreader& br) {
	br.read_n_bits(8);

	std::array<uint8_t, 4> pixel;
	pixel[0] = br.read_n_bits(8);
	pixel[1] = br.read_n_bits(8);
	pixel[2] = br.read_n_bits(8);
	pixel[3] = last_pixel[3];

	last_pixel = pixel;
	update_seen();
}

void qoi_op_RGBA(bitreader& br) {
	br.read_n_bits(8);

	std::array<uint8_t, 4> pixel;
	pixel[0] = br.read_n_bits(8);
	pixel[1] = br.read_n_bits(8);
	pixel[2] = br.read_n_bits(8);
	pixel[3] = br.read_n_bits(8);

	last_pixel = pixel;
	update_seen();
}

void qoi_op_INDEX(bitreader& br) {
	br.read_n_bits(2);

	uint8_t index = br.read_n_bits(6);
	last_pixel = seen[index];
	update_seen();
}

uint8_t wrap_around(int32_t val) {
	if (val < 0) {
		return 256 + val;
	}
	else if (val > 255) {
		return val - 256;
	}
	else {
		return val;
	}
}
void qoi_op_DIFF(bitreader& br) {
	br.read_n_bits(2);

	std::array<uint8_t, 4> pixel;
	int8_t d_r = br.read_n_bits(2)-2;
	int8_t d_g = br.read_n_bits(2)-2;
	int8_t d_b = br.read_n_bits(2)-2;

	pixel[0] = wrap_around(last_pixel[0] + d_r);
	pixel[1] = wrap_around(last_pixel[1] + d_g);
	pixel[2] = wrap_around(last_pixel[2] + d_b);
	pixel[3] = last_pixel[3];

	last_pixel = pixel;
	update_seen();
}
void qoi_op_LUMA(bitreader& br) {
	br.read_n_bits(2);

	std::array<uint8_t, 4> pixel;
	int8_t dg = br.read_n_bits(6) - 32;
	int8_t dr_dg = br.read_n_bits(4) - 8;
	int8_t db_dg = br.read_n_bits(4) - 8;

	pixel[0] = wrap_around(dr_dg+dg+last_pixel[0]);
	pixel[1] = wrap_around(last_pixel[1] + dg);
	pixel[2] = wrap_around(db_dg + dg + last_pixel[2]);
	pixel[3] = last_pixel[3];

	last_pixel = pixel;
	update_seen();
}
uint8_t qoi_op_RUN(bitreader& br) {
	br.read_n_bits(2);

	return static_cast<uint8_t>(br.read_n_bits(6))+1;
}

int read_next_chunk(std::ifstream& is, bitreader& br) {
	uint8_t tag = is.peek();
	if (tag == 254) {
		qoi_op_RGB(br);
	}
	else if (tag == 255) {
		qoi_op_RGBA(br);
	}
	else if ((tag >> 6 & 3) == 1) {
		qoi_op_DIFF(br);
	}
	else if ((tag >> 6 & 3) == 2) {
		qoi_op_LUMA(br);
	}
	else if ((tag >> 6 & 3) == 3) {
		return qoi_op_RUN(br);
	}
	else if ((tag >> 6 & 3) == 0) {
		qoi_op_INDEX(br);
	}
	return 1;
}
int decompress(std::ifstream& is, std::ofstream& os) {
	//read header
	std::string header(4, ' ');
	is.read(header.data(), 4);
	if (header != "qoif") {
		std::print("Error: file should be 'qoif', not {}", header);
		return EXIT_FAILURE;
	}

	bitreader br(is);
	uint32_t w = br.read_n_bits(32);
	uint32_t h = br.read_n_bits(32);
	uint8_t c = br.read_n_bits(8);
	br.read_n_bits(8);				//colospace info
	if (c != 4) {
		std::print("Warning: color channels should be 4, but are {}", c);
	}

	//create image
	image<std::array<uint8_t, 4>> img(w, h, c);

	//start reading
	size_t pixel_counter = 0;
	while (pixel_counter < img.size()) {
		uint8_t run = read_next_chunk(is, br);
		while (run--> 0) {
			img.img_.push_back(last_pixel);

			pixel_counter++;
			if (pixel_counter >= img.size()) break;
		}
	}

	//write pam image
	os << "P7\nWIDTH " << img.width() << "\nHEIGHT " << img.height() <<
		"\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
	os.write(img.rawdata(), img.rawsize());

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::print("Error: program accepts 2 parmaters, not {}", argc - 1);
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	std::ofstream os(argv[2], std::ios::binary);
	if (!is or !os) {
		std::print("Error: fail managing files.");

		return EXIT_FAILURE;
	}

	return decompress(is, os);
}
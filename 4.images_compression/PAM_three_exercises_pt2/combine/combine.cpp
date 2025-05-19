#include <cstdio>
#include <iostream>
#include <fstream>
#include <expected>

#include <string>
#include <array>
#include <vector>

template<typename T>
struct image {
	int r_;
	int c_;
	int d_;
	std::vector<T> image_;

	image(int r, int c, int d) : r_(r), c_(c), d_(d) {
		image_.resize(r * c);
	}

	int width() { return c_; }
	int height() { return r_; }
	int depth() { return d_; }


	char* rawimage() {
		return reinterpret_cast<char*>(image_.data());
	}
	char* rawimage() const {
		return reinterpret_cast<const char*>(image_.data());
	}

	int rawsize() const {
		return r_ * c_ * sizeof(T);
	}

};

std::expected<image<char>, std::string>
read_header(std::ifstream& is) {
	std::string field;
	std::string value;

	//check file type
	is >> field;
	if (field != "P7") {
		return std::unexpected("Error: file format is not PAM.\n");
	}

	// read image info
	int r = -1;
	int c = -1;
	int d = -1;
	int maxval = -1;
	while (1) {
		is >> field;
		if (field == "ENDHDR") {
			is.get();
			break;
		}
		else if (field[0] == '#') {
			std::string dummy;
			std::getline(is, dummy);
			continue;
		}

		is >> value;
		if (field == "WIDTH") {
			c = std::stoi(value);
		}
		else if (field == "HEIGHT") {
			r = std::stoi(value);
		}
		else if (field == "DEPTH") {
			d = std::stoi(value);
		}
		else if (field == "MAXVAL") {
			maxval = std::stoi(value);
		}
	}

	if ((c * r * d < 0) or (maxval < 0)) {
		return std::unexpected("Error: not enough info in PAM heaer");
	}

	//create image structure
	if (d != 1) {
		return std::unexpected("Error: program axepected a grayscale image, not an RGB image.\n");
	}
	if (maxval > 255) {
		return std::unexpected("Error: program axepected an image with pixel maxval <=255.\n");
	}
	image<char> im(r, c, d);
	return im;
}

void print_header_rgb(image < std::array<uint8_t, 3>>& im, std::ofstream& os) {
	os << "P7\n";
	os << "WIDTH " << im.width() << "\n";
	os << "HEIGHT " << im.height() << "\n";
	os << "DEPTH " << 3 << "\n";
	os << "MAXVAL " << "255" << "\n";
	os << "TUPLTYPE " << "RGB" << "\n";
	os << "ENDHDR\n";
}

void print_header_grayscale(image < std::array<uint8_t, 3>>& im, std::ofstream& os) {
	os << "P7\n";
	os << "WIDTH " << im.width() << "\n";
	os << "HEIGHT " << im.height() << "\n";
	os << "DEPTH " << "1" << "\n";
	os << "MAXVAL " << "255" << "\n";
	os << "TUPLTYPE " << "GRAYSCALE" << "\n";
	os << "ENDHDR\n";
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Error: program accepts one parameter, not " << argc - 1 << ".\n";
		return EXIT_FAILURE;
	}
	std::string filename(argv[1]);

	//read _R.pam file
	std::ifstream is_red(filename+"_R.pam", std::ios::binary);
	if (!is_red) {
		std::cerr << "Error: fail opening file " << filename + "_R.pam" << ".\n";
		return EXIT_FAILURE;
	}

		//read and check header
		auto res = read_header(is_red);
		if (!res) {
			std::cerr << res.error();
			return EXIT_FAILURE;
		}
		image<char> im_red = res.value();
		
		//read image
		is_red.read(im_red.rawimage(), im_red.rawsize());

	//read _G.pam file
	std::ifstream is_green(filename + "_G.pam", std::ios::binary);
	if (!is_green) {
		std::cerr << "Error: fail opening file " << filename + "_G.pam" << ".\n";
		return EXIT_FAILURE;
	}

		//read and check header
		res = read_header(is_green);
		if (!res) {
			std::cerr << res.error();
			return EXIT_FAILURE;
		}
		image<char> im_green = res.value();
		
		//read image
		is_green.read(im_green.rawimage(), im_green.rawsize());

	//read _B.pam file
	std::ifstream is_blue(filename + "_B.pam", std::ios::binary);
	if (!is_blue) {
		std::cerr << "Error: fail opening file " << filename + "_B.pam" << ".\n";
		return EXIT_FAILURE;
	}

		//read and check header
		res = read_header(is_blue);
		if (!res) {
			std::cerr << res.error();
			return EXIT_FAILURE;
		}
		image<char> im_blue = res.value();

		//read image
		is_blue.read(im_blue.rawimage(), im_blue.rawsize());

	
	//print output file header
	std::ofstream os_reconstructed(filename + "_reconstructed.pam", std::ios::binary);
	if (!os_reconstructed) {
		std::cerr << "Error: fail opening file " << filename + "_reconstructed.pam" << ".\n";
		return EXIT_FAILURE;
	}

	image<std::array<uint8_t, 3>> im_rgb(im_red.height(), im_red.width(), 3);
	char* rawimage = im_rgb.rawimage();
	for (size_t i = 0; i < im_red.rawsize(); i++) {
		*(im_rgb.rawimage() + i*3) = *(im_red.rawimage() + i);
		*(im_rgb.rawimage() + i*3 +1) = *(im_green.rawimage() + i);
		*(im_rgb.rawimage() + i*3 + 2) = *(im_blue.rawimage() + i);
	}
	print_header_rgb(im_rgb, os_reconstructed);
	os_reconstructed.write(im_rgb.rawimage(), im_rgb.rawsize());

	return EXIT_SUCCESS;
}
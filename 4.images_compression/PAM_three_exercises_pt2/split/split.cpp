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

	int width()		{ return c_; }
	int height()	{ return r_; }
	int depth()		{ return d_; }


	char* rawimage() {
		return reinterpret_cast<char *>(image_.data());
	}
	char* rawimage() const {
		return reinterpret_cast<const char*>(image_.data());
	}
	int rawsize() const{
		return r_ * c_ * sizeof(T);
	}

};

std::expected<image<std::array<uint8_t, 3>>, std::string>
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

	if ((c * r * d  < 0) or (maxval < 0)){
		return std::unexpected("Error: not enough info in PAM heaer");
	}

	//create image structure
	if (d != 3) {
		return std::unexpected("Error: program axepected a color image, not a grayscale image.\n");
	}
	if (maxval > 255) {
		return std::unexpected("Error: program axepected an image with pixel maxval <=255.\n");
	}
	image<std::array<uint8_t,3>> im(r, c, d);
	return im;
}

void print_header(image < std::array<uint8_t, 3>>& im, std::ofstream& os) {
	os << "P7\n";
	os << "WIDTH " << im.width() <<"\n";
	os << "HEIGHT " << im.height() << "\n";
	os << "DEPTH " << im.depth() << "\n";
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

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cerr << "Error: fail opening file " << argv[1] << ".\n";
		return EXIT_FAILURE;
	}

	//read and check header
	auto res = read_header(is);
	if (!res) {
		std::cerr << res.error();
		return EXIT_FAILURE;
	}
	image<std::array<uint8_t, 3>> im = res.value();


	//read image
	is.read(im.rawimage(), im.rawsize());

	//isolate file name
	std::string inputfile(argv[1]);
	std::string extension(".pam");

	auto it = inputfile.find(extension);
	inputfile.erase(it, it + extension.size());

	//prepare rawimage
	const char* rawimage = im.rawimage();

	//extract red in grayscale
	std::ofstream os_red(inputfile + "_R.pam", std::ios::binary);
	if (!os_red) {
		std::cerr << "Error: fail opening file " << std::string(argv[1]) + "_R.pam" << ".\n";
		return EXIT_FAILURE;
	}

	print_header_grayscale(im, os_red);
	for (int i = 0; i < im.rawsize(); i += 3) {
		os_red.write(rawimage + i, 1);
	}

	//extract green in grayscale
	std::ofstream os_green(inputfile + "_G.pam", std::ios::binary);
	if (!os_green) {
		std::cerr << "Error: fail opening file " << inputfile + "_G.pam" << ".\n";
		return EXIT_FAILURE;
	}

	print_header_grayscale(im, os_green);
	for (int i = 1; i < im.rawsize(); i += 3) {
		os_green.write(rawimage + i, 1);
	}

	//extract blue in grayscale
	std::ofstream os_blue(inputfile + "_B.pam", std::ios::binary);
	if (!os_green) {
		std::cerr << "Error: fail opening file " << inputfile + "_B.pam" << ".\n";
		return EXIT_FAILURE;
	}

	print_header_grayscale(im, os_blue);
	for (int i = 2; i < im.rawsize(); i += 3) {
		os_blue.write(rawimage + i, 1);
	}

	return EXIT_SUCCESS;
}
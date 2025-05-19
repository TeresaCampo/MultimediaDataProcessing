#include <iostream>
#include <fstream>
#include <cstdio>

#include <cmath>

#include <vector>
#include <string>

int bigendian_reader(std::ifstream& is, int nbytes) {
	int val = 0;

	val = (val << 8) | (is.get() & 0xff);
	if (nbytes > 1)	val = (val << 8) | (is.get() & 0xff);
	if (nbytes > 2)	val = (val << 8) | (is.get() & 0xff);
	if (nbytes > 3)	val = (val << 8) | (is.get() & 0xff);

	return val;
}
struct image {
	int w_ = 0;
	int h_ = 0;
	int d_ = 0;
	bool RGB_ = true;
	std::vector<std::vector<std::vector<int>>> i_;

	image(int w, int h, int d, bool RGB) : w_(w), h_(h), d_(d), RGB_(RGB) {
		auto pixel = std::vector<int>(d_);
		auto row = std::vector<std::vector<int>>(w_, pixel);
		i_.resize(h_, row);	
	};

	int& operator()(int r, int c, int ch) {
		return i_[r][c][ch];
	}
};


int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Error: 2 parameters needed.\n";
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cerr << "Error: fail opening input file.\n";
		return EXIT_FAILURE;
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::cerr << "Error: fail opening output file.\n";
		return EXIT_FAILURE;
	}

	//read input header, copy it in as output header
	std::string t;
	std::string v;
	int w = 0;
	int h = 0;
	int d = 0;
	int maxval = 0;
	bool RGB = 0;

	is >> t;
	if (t != "P7") {
		std::cerr << "Error: data is not compressed with .pam\n";
		return EXIT_FAILURE;
	}
	os << t << "\n";

	while (1) {
		is >> t;
		if (t[0] == '#') {
			std::string comment;
			std::getline(is, comment, '\n');
			os << "#" << comment << "\n";
		}

		if (t == "ENDHDR") {
			os << t << "\n";
			is.get(); //read following space

			break;
		}
		is >> v;

		os << t << " ";
		os << v << "\n";
		if (t == "WIDTH")	w = std::stoi(v);
		if (t == "HEIGHT")	h = std::stoi(v);
		if (t == "DEPTH")	d = std::stoi(v);
		if (t == "MAXVAL")	maxval = std::stoi(v);
		if (t == "TUPLTYPE") {
			if (v == "RGB") RGB = true;
			else RGB = false;
		}
	}

	//read input image
	image i(w, h, d, RGB);
	int sample_nbytes = static_cast<int>(ceil(ceil(log2(maxval)) / 8));

	for (int r = 0; r < h; r++) {
		for (int c = 0; c < w; c++) {
			for (int ch = 0; ch < d; ch++) {
				int val = 0;
				is.read(reinterpret_cast<char*>(&val), sample_nbytes);

				i(r, c, ch) = val;
			}
		}
	}

	//write output image
	for (int r = 0; r < h; r++) {
		for (int c = w - 1; c >= 0; c--) {
			for (int ch = 0; ch < d; ch++) {
				int val = i(r, c, ch);

				os.write(reinterpret_cast<char*>(&val), sample_nbytes);
			}
		}
	}
	return EXIT_SUCCESS;
}
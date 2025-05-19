#include <cstdio>
#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <cmath>

void write_int_bigendinan(std::ofstream& os,int n, int nbytes) {
	if (nbytes > 3) os.put((n >> 24) & 0xff);
	if (nbytes > 2) os.put((n >> 16) & 0xff);
	if (nbytes > 1) os.put((n >> 8) & 0xff);
	os.put((n >> 0) & 0xff);
}
struct bitreader {
	std::ifstream& is_;
	char buffer_ = 0;
	int n_ = 0;

	bitreader(std::ifstream& is) : is_(is), n_(0), buffer_(0) {};
	int read_1_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}
	int read_n_bit(int nbit) {
		int val = 0;
		while (nbit--> 0) {
			val = (val << 1) | read_1_bit();
		}

		return val;
	}
};
struct image {
	int d_ = 0;
	int w_ = 0;
	int h_ = 0;
	bool RGB_ = 0;
	std::vector<std::vector<int>> i_;
	
	image(int d, int w, int h, bool RGB) : d_(d), w_(w), h_(h), RGB_(RGB) {
		std::vector<int> rows(w_, 0);
		i_.resize(h_, rows );	
	};
	int& operator()(int r, int c) {
		return i_[r][c];
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

	//header info
	std::string t(' ', 8);
	std::string v(' ', 8);

	int w = 0;
	int h = 0;
	int d = 0;
	int maxval = 0;
	bool RGB = 0;

	is >> t;
	if (t != "P7") {
		std::cerr << "Error: file is not .pam\n";
		return EXIT_FAILURE;
	}
	os.write(t.data(), t.length());
	os.put('\n');


	while (1) {
		is >> t;

		//if is a comment, copy all the line in output file
		if (t[0] == '#') {
			std::string comment;
			std::getline(is, comment, '\n');
			os.put('#');
			os.write(comment.data(), comment.length());
			os.put('\n');

			continue;
		}

		//if is not a comment
		//end of header, copy it and break
		if (t == "ENDHDR") {
			os.write(t.data(), t.length());
			os.put('\n');

			is.get();
			break;
		}
		//command with value, copy it
		is >> v;
		os.write(t.data(), t.length());
		os.put(' ');

		os.write(v.data(), v.length());
		os.put('\n');

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
	image i= image(d, w, h, RGB);
	bitreader br(is);
	int sample_nbytes = ceil(ceil(log2(maxval)) / 8);

	for (int r = 0; r < h; r++ ) {
		for (int c = 0; c < w; c++) {
			i(r, c) = br.read_n_bit(sample_nbytes * 8);
		}
	}
	
	//write output image
	for (int r = h-1; r >= 0; r--) {
		for (int c = 0; c < w; c++) {
			write_int_bigendinan(os, i(r, c), sample_nbytes);
		}
	}

	return EXIT_SUCCESS;	
}
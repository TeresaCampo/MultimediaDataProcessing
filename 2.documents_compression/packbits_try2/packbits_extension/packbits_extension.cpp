#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

int compressor(std::string input, std::string output) {
	std::ifstream inFile(input, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: not able to open " << input;
		return EXIT_FAILURE;
	}

	std::ofstream outFile(output, std::ios::binary);
	if (!outFile) {
		std::cerr << "Error: not able to open " << output;
		return EXIT_FAILURE;
	}

	std::vector<char> v;
	bool run = 0;
	char val;

	//i'm in a run
	//i'm in a copy
	//it's first char
	//it's second char and i decide if I am in a run or a copy
	while (inFile.get(val)) {
		v.push_back(val);

		if (v.size() == 2) {
			if (v[0] == v[1]) run = 1;
			else run = 0;
			continue;
		}
		if (v.size() > 2) {
			if (run and val != v[0] and v.size()<=128)
			{
				//need to write number of occurrences (v.size()-1)
				//need to write v, without last added element
				//need to shift v
				unsigned char r = 257 - (v.size() - 1);
				outFile.put(r);

				outFile.write(v.data(), 1);

				v.clear();
				v.push_back(val);
				continue;
			}
			if (!run and val == v[v.size() - 2] and val ==v[v.size()-3])
			{
				//need to write number of occurrences (v.size()-2)
				//need to write v, without last 2 added elements
				//need to shift v
				unsigned char r = v.size() - 3 - 1;
				outFile.put(r);

				outFile.write(v.data(), v.size() - 3);

				v.clear();
				v.push_back(val);
				v.push_back(val);
				v.push_back(val);
				run = 1;
				continue;
			}
		}
		if (v.size() == 130) {
			if (run) {
				unsigned char r = 257 - 128;
				outFile.put(r);

				outFile.write(v.data(), 1);
				char pre_val = v[v.size() - 2];
				v.clear();
				v.push_back(pre_val);
				v.push_back(val);

				if (v[0] == v[1]) run = 1;
				else run = 0;
			}
			else {
				unsigned char r = 128 - 1;
				outFile.put(r);

				outFile.write(v.data(), 128);
				char pre_val = v[v.size() - 2];
				v.clear();
				v.push_back(pre_val);
				v.push_back(val);

				if (v[0] == v[1]) run = 1;
				else run = 0;
			}
		}
	}
	//print if there is something in v that has not been printed
	if (v.size() > 0) {
		if (run) {
			unsigned char r = 257 - v.size();
			outFile.put(r);

			outFile.write(v.data(), 1);
		}
		else {
			unsigned char r = v.size() - 1;
			outFile.put(r);

			outFile.write(v.data(), v.size());
		}
	}
	//print EOF
	outFile.put(128);

	return EXIT_SUCCESS;
}

int decompressor(std::string input, std::string output) {
	std::ifstream inFile(input, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: not able to open " << input;
		return EXIT_FAILURE;
	}

	std::ofstream outFile(output, std::ios::binary);
	if (!outFile) {
		std::cerr << "Error: not able to open " << output;
		return EXIT_FAILURE;
	}

	char signed_r;
	unsigned char r;
	char val;
	std::vector<char> v;

	while (inFile.get(signed_r)) {
		r = static_cast<unsigned char>(signed_r);

		if (r == 128) break;
		if (r > 128) //run
		{
			r = 257 - r;
			inFile.get(val);

			v.insert(v.begin(), r, val);
			outFile.write(v.data(), v.size());
			v.clear();
			continue;
		}
		if (r < 128) //copy
		{
			r = r + 1;
			while (r-- > 0) {
				inFile.get(val);
				outFile.put(val);
			}
			continue;
		}
	}
	return EXIT_SUCCESS;
}
int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Error: program accepts 3 parameters.\n";
		return EXIT_FAILURE;
	}
	if (std::string(argv[1]) == "c") {
		return compressor(argv[2], argv[3]);
	}
	if (std::string(argv[1]) == "d") {
		return decompressor(argv[2], argv[3]);
	}
	else {
		std::cout << "Error: first parameter should be 'c' or 'd'.\n";
		return EXIT_FAILURE;
	}
}
#include "mat.h"
#include "pgm.h"
#include <fstream>
#include <cstdlib>
#include <print>

#include <string>

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file.");
		return false;
	}

	//read STREAM-HEADER
	std::string stream_header(9, ' ');
	is.read(stream_header.data(), 9);
	if (stream_header != "YUV4MPEG2") {
		std::println("Error: input file is not .y4m format.");
		return false;
	}

	//read TAGGED-FIELD
	int r = 0;
	int c = 0;
	while (1) {
		char separator = is.get();
		if (separator == 0x0A) break;
		else if (separator != 0x20) {
			std::println("Error: wrong header format.");
			return false;
		}
		char tag = is.get();
		if (tag == 'W') {
			std::string width;
			is >> width;
			c = std::stoi(width);
		}
		else if (tag == 'H') {
			std::string heigth;
			is >> heigth;
			r = std::stoi(heigth);
		}
		else if (tag == 'I') {
			char interlacing = is.get();
			if (interlacing != 'p') {
				std::println("Error: input file is not encoded with 'progressive interlacing'");
				return false;
			}
		}
		else if (tag == 'C') {
			std::string color_plane;
			is >> color_plane;
			if (color_plane != "420jpeg") {
				std::println("Error: input file is not encoded with '420jpeg'");
				return false;
			}
		}
		else {
			//ignore tag field value
			std::string dummy;
			is >> dummy;
		}
	}
	
	//read frames untill when they are present
	while (!is.eof()) {
		//read FRAME-HEADER
		std::string frame_header(5, ' ');
		is.read(frame_header.data(), 5);
		if (frame_header != "FRAME") {
			break;
		}

		//read and ingore all the TAGGED-FIELDs
		while (1) {
			char separator = is.get();
			if (separator == 0x0A) break;
			else if (separator != 0x20) {
				std::println("Error: wrong header format.");
				return false;
			}

			std::string dummy;
			is >> dummy;
		}

		//read all Y pixels
		mat<uint8_t> image(r, c);
		is.read(image.rawdata(), image.rawsize());

		frames.push_back(image);
		//ingore all Cb and Cr values
		mat<uint8_t> image_cb(r/2, c/2);
		is.read(image_cb.rawdata(), image_cb.rawsize());

		mat<uint8_t> image_cr(r / 2, c / 2);
		is.read(image_cr.rawdata(), image_cr.rawsize());
	}
	return true;
}

/*
int main(int argc, char** argv) {
	std::vector<mat<uint8_t>> frames;

	y4m_extract_gray(argv[1], frames);
	std::string output_name("test");

	int counter = 1;
	for (const auto& frame : frames) {
		save_pgm(output_name + std::to_string(counter)+ ".pgm", frame, false);
	}
}
*/


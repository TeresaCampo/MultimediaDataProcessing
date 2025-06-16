#include "types.h"
#include "mat.h"
#include "ppm.h"
#include <fstream>
#include <cstdlib>
#include <print>

#include <string>

template<typename T>
uint8_t saturate(T v, int lb, int ub) {
	if (v < lb) return lb;
	else if (v > ub) return ub;
	else return static_cast<char>(v);
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
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

		//read Y pixels
		mat<uint8_t> image_y(r, c);
		is.read(image_y.rawdata(), image_y.rawsize());

		//read Cb and Cr values
		mat<uint8_t> image_cb(r / 2, c / 2);
		is.read(image_cb.rawdata(), image_cb.rawsize());

		mat<uint8_t> image_cr(r / 2, c / 2);
		is.read(image_cr.rawdata(), image_cr.rawsize());

		//create RGB image
		mat<vec3b> image(r, c);
		for (int r_index = 0; r_index < image.rows(); r_index++) {
			for (int c_index = 0; c_index < image.cols(); c_index++) {
				uint8_t Y = saturate<uint8_t>(image_y(r_index, c_index), 16, 235);
				uint8_t Cb = saturate<uint8_t>(image_cb(r_index / 2, c_index / 2), 16, 240);
				uint8_t Cr = saturate<uint8_t>(image_cr(r_index / 2, c_index / 2), 16, 240);

				double R = (Y - 16) * 1.164 + (Cb - 128) * 0		+ (Cr - 128) * 1.596;
				double G = (Y - 16) * 1.164 + (Cb - 128) * (-0.392) + (Cr - 128) * (-0.813);
				double B = (Y - 16) * 1.164 + (Cb - 128) * 2.017 + (Cr - 128) * 0;
				image(r_index, c_index)[0] = saturate<double>(R ,0, 255);
				image(r_index, c_index)[1] = saturate<double>(G ,0, 255);
				image(r_index, c_index)[2] = saturate<double>(B ,0, 255);			
			}
		}
		frames.push_back(image);
	}
	return true;
}

/*
int main(int argc, char** argv) {
	std::vector<mat<vec3b>> frames;

	y4m_extract_color(argv[1], frames);
	std::string output_name("test");

	int counter = 1;
	for (const auto& frame : frames) {
		save_ppm(output_name + std::to_string(counter) + ".pgm", frame, false);
	}
}
*/
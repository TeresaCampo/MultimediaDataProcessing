#include <cstdio>
#include <print>
#include <fstream>
#include <bit>
#include <cmath>
#include <algorithm>

#include <string>
#include <vector>
#include <array>

template<typename T>
bool read_be(std::ifstream& is, uint32_t nbytes, T& dest) {
	is.read(reinterpret_cast<char*>(&dest), nbytes);
	dest = std::byteswap(dest);

	return is.good();
}
struct bitreader {
	uint64_t buffer_;
	uint32_t n_;

	bitreader(uint64_t val) {
		buffer_ = val;
		n_ = 64;
	}

	uint64_t read_one_bit() {
		n_--;
		return buffer_ >> n_ & 1;
	}

	uint64_t read_n_bits(uint8_t nbits) {
		uint64_t val = 0;
		while (nbits-- > 0) {
			val = val << 1 | read_one_bit();
		}
		return val;
	}
};
struct channel_state {
	std::vector<int16_t> history_;
	std::vector<int16_t> weights_;

	channel_state(std::ifstream& is) {
		history_.resize(4);
		for (int i = 0; i < 4; i++) {
			read_be(is, 2, history_[i]);
		}
		weights_.resize(4);
		for (int i = 0; i < 4; i++) {
			read_be(is, 2, weights_[i]);
		}
	}

	std::vector<int16_t>& history() { return history_; }
	std::vector<int16_t>& weights() { return weights_; }
};	
struct channel_slices {
	std::vector<uint64_t> slices_;

	channel_slices(std::ifstream& is, uint16_t samples_per_channel) {
		slices_.resize(static_cast<uint32_t>(std::ceil(samples_per_channel/20)));
		for (int i = 0; i < slices_.size(); i++) {
			read_be(is, 8, slices_[i]);
		}
	}

	uint32_t num_slices() {
		return static_cast<uint32_t>(slices_.size());
	}
	uint64_t& num(uint32_t pos) {
		return slices_[pos];
	}
};

static const std::array<double, 8> dequant_tab = { 0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7 };
struct slice{
	uint64_t slice_ = 0;
	uint64_t sf_ = 0; //scalefactor
	std::vector<uint8_t> qz_residuals_;
	std::vector<int16_t> samples_;

	slice(uint64_t slice, channel_state& state,uint32_t slice_size = 20) {
		slice_ = slice;
		
		bitreader br(slice_);
		sf_ = br.read_n_bits(4);
		
		for (int i = 0; i < slice_size; i++) {
			qz_residuals_.push_back( static_cast<uint8_t>(br.read_n_bits(3)));
		}

		//dequantize scale factor
		sf_ = static_cast<uint64_t>(std::round(std::pow(sf_ + 1, 2.75)));

		//for each residual
		for (int i = 0; i < slice_size; i++) {
			//modify residual
			double r = sf_ * dequant_tab[qz_residuals_[i]];
			r = (r < 0) ? std::ceil(r - 0.5) : std::floor(r + 0.5);

			//calculate prediction with states, history and weights
			int64_t p = 0;
			for (int c = 0; c < 4; c++) {
				p += (state.history_[c] * state.weights_[c]);
			}
			p >>= 13;

			//add p and r
			p += static_cast<int64_t>(r);
			if (p < INT16_MIN) p = INT16_MIN;
			if (p > INT16_MAX) p = INT16_MAX;
			samples_.push_back(static_cast<int16_t>(p));

			//update state, history and weights
			int16_t delta = static_cast<int16_t>(static_cast<int64_t>(r) >> 4);
			for (int c = 0; c < 4; c++) {
				state.weights_[c] += (state.history_[c] < 0) ? -delta : delta;
			}
			for (int c = 0; c < 3; c++) {
				state.history_[c] = state.history_[c + 1];
			}
			state.history_[3] = static_cast<int16_t>(p);
		}
	}
};
void merge_channels(std::vector<int16_t>& sc1, std::vector<int16_t>& sc2, std::vector<int16_t>& m) {
	for (int i = 0; i < sc2.size(); i++) {
		m.push_back(sc1[i]);
		m.push_back(sc2[i]);
	}
}

void write_wav_format(std::ofstream& os, std::vector<int16_t> samples) {
	os << "RIFF";
	uint32_t val = static_cast<uint32_t>(samples.size())*2 + 44 - 8;
	os.write(reinterpret_cast<char*>(&val), 4);
	os << "WAVE";
	os << "fmt";
	os.put(32);
	val = 16;
	os.write(reinterpret_cast<char*>(&val), 4);
	val = 1;
	os.write(reinterpret_cast<char*>(&val), 2);
	val = 2;
	os.write(reinterpret_cast<char*>(&val), 2);
	val = 44100;
	os.write(reinterpret_cast<char*>(&val), 4);
	val = (44100 * 16 * 2) / 8;
	os.write(reinterpret_cast<char*>(&val), 4);
	val = 4;
	os.write(reinterpret_cast<char*>(&val), 2);
	val = 16;
	os.write(reinterpret_cast<char*>(&val), 2);
	os << "data";
	val = static_cast<uint32_t>(samples.size());
	os.write(reinterpret_cast<char*>(&val), 4);

	os.write(reinterpret_cast<char*>(samples.data()), samples.size()*2);
}

void decompress(std::ifstream& is, std::ofstream& os) {
	//file header
	std::string magic(4, ' ');
	is.read(reinterpret_cast<char*>(magic.data()), 4);
	if (magic != "qoaf") {
		std::println("Error: wrong magic number-> {}", magic);
		return;
	}
	uint32_t samples;
	read_be(is, 4, samples);

	std::vector<int16_t> decompressed;

	//for each frame
	double tmp = std::ceil(samples / (256 * 20.0));
	size_t nframes = static_cast<size_t>(tmp);
	while (nframes-- > 0) {
		//frame header
		uint8_t num_channels = is.get();
		uint32_t samplerate = 0;
		uint16_t fsamples = 0;
		uint16_t fsize;

		read_be(is, 3, samplerate);
		samplerate = samplerate >> 8;
		read_be(is, 2, fsamples);
		read_be(is, 2, fsize);

		//state per channel
		channel_state c1_state(is);
		channel_state c2_state(is);

		//slices per channel 
		if ((nframes != 0) and (fsamples / 20 != 256)) {
			std::println("Error: only last frame can contain less than 256 slices per channel");
			return;
		}
		int nslices = static_cast<int>(std::ceil(fsamples / 20.0))*2;
		
		for (int i = 0; i < nslices; i= i+2) {
			if (nframes == 0 and i == nslices - 2) {
				size_t delta = is.end - is.tellg();
				if (delta / 2 < 8) {
					uint64_t slice1_ = 0;
					read_be(is, delta / 2, slice1_);
					slice s1(slice1_, c1_state, delta / 2);

					uint64_t slice2_ = 0;
					read_be(is, delta / 2, slice2_);
					slice s2(slice2_, c2_state, delta / 2);

					merge_channels(s1.samples_, s2.samples_, decompressed);
					continue;
				}
			}
			uint64_t slice_ = 0;
			read_be(is, 8, slice_);
			slice s1(slice_, c1_state);

			read_be(is, 8, slice_);
			slice s2(slice_, c2_state);
			merge_channels(s1.samples_, s2.samples_, decompressed);
		}
	}
	
	//produce .wav
	write_wav_format(os, decompressed);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println("Error: program accepts 2 parameters, not {}.", argc - 1);
		return EXIT_FAILURE;
	}
	//input file
	std::string input(argv[1]);
	if (input.substr(input.size() - 4) != ".qoa") {
		std::println("Error: first parameter should be a .qoa file.");
		return EXIT_FAILURE;
	}
	std::ifstream is(input, std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}.", argv[1]);
		return EXIT_FAILURE;
	}

	//output file
	std::string output(argv[2]);
	if (output.substr(output.size() - 4) != ".wav") {
		output += ".wav";
	}
	std::ofstream os(output, std::ios::binary);

	decompress(is, os);
}

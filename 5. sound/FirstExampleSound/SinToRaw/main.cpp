#include <numbers>
#include <cmath>
#include <vector>
#include <fstream>

int main() {
	// y = A *sin(2*pi*f*t)
	// freq. camp. = 44100 Hz
	// bit per sample = 16 bits

	double duration = 3;	//s
	double f = 440;			//Hz, sin() signal frequency
	double A = 32767;
	std::numbers::pi;

	std::vector<int16_t> samples(static_cast<int16_t>(duration * 44100));
	for (size_t i = 0 ; i < samples.size(); i++) {
		samples[i] = static_cast<int16_t>(A * std::sin(2 * std::numbers::pi * f * i/44100));
	}

	// saving format
	std::ofstream os("audio1.raw", std::ios::binary);
	os.write(reinterpret_cast<char*>(samples.data()), samples.size() * 2);
}
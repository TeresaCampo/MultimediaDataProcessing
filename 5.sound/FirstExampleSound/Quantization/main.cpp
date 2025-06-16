#include <iostream>
#include <fstream>
#include <cstdio>

#include <vector>

int16_t saturate(int32_t value) {
	if (value > 32767) {
		return 32767;
	}
	else if (value < -32767) {
		return -32767;
	}
	else {
		return value;
	}
}

int main() {
	std::ifstream is("test.raw", std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	std::ofstream os("audio_test.raw", std::ios::binary);

	//read audio
	is.seekg(0, std::ios::end);
	size_t size = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<int16_t> samples(size / 2);
	is.read(reinterpret_cast<char*>(samples.data()), size);

	//quantize
	for (auto& s : samples) {
		s = (s /1000) *1000;
	}

	//write .raw
	os.write(reinterpret_cast<char*>(samples.data()), samples.size() * 2);
}
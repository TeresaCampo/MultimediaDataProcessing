#include <fstream>
#include <print>
#include <cmath>
#include <numbers>

#include <vector>
#include <map>


void load_raw_audio(std::ifstream& is, std::vector<int16_t>& samples) {
	is.seekg(0, is.end);
	size_t file_size = is.tellg();
	is.seekg(0, is.beg);

	samples.resize(file_size / 2);
	is.read(reinterpret_cast<char*>(samples.data()), file_size);
}
template<typename T>
void save_raw_audio(std::ofstream& os, std::vector<T>& samples) {
	os.write(reinterpret_cast<char*>(samples.data()), samples.size() * sizeof(T));
}

template<typename T>
double entropy(std::vector<T>& samples) {
	std::map<T, double> f;
	for (const auto& s : samples) {
		f[s] += 1;
	}

	double entropy = 0;
	for (const auto& [sample, frequency] : f) {
		double p = frequency / samples.size();
		entropy -= p * std::log2(p);
	}

	return entropy;
}
int quantization_test(std::ifstream& is) {
	//step 1
	std::vector<int16_t> samples;
	load_raw_audio(is, samples);

	double entropy1 = entropy(samples);

	//step 2
	std::vector<int16_t> samples_quantized;
	double Q = 2600;
	for (const auto& s : samples) {
		int16_t s_quantized = static_cast<int16_t>(std::round(s / Q));
		samples_quantized.push_back(s_quantized);
	}

	//step 3
	double entropy2 = entropy(samples_quantized);
	std::println("Original samples' entropy-> {}\nQuantized samples' entropy-> {}", entropy1, entropy2);

	//step 4
	std::vector<int16_t> samples_reconstructed;
	for (const auto& s : samples_quantized) {
		int16_t s_reconstructed = static_cast<int16_t>(round(s * Q));
		samples_reconstructed.push_back(s_reconstructed);
	}

	std::ofstream os_qt("output_qt.raw", std::ios::binary);
	if (!os_qt) {
		std::print("Error: fail opening output file {}.", "output_qt.raw");
		return EXIT_FAILURE;
	}
	save_raw_audio(os_qt, samples_reconstructed);
	double entropy3 = entropy(samples_reconstructed);

	//step 5
	std::vector<int16_t> errors;
	for (int i = 0; i < samples.size(); i++) {
		int16_t error = samples[i] - samples_reconstructed[i];
		errors.push_back(error);
	}
	std::ofstream os_error("error_qt.raw", std::ios::binary);
	if (!os_error) {
		std::print("Error: fail opening output file {}.", "error_qt.raw");
		return EXIT_FAILURE;
	}
	save_raw_audio(os_error, errors);
	return EXIT_SUCCESS;
}

int MDCT_test(std::ifstream& is) {
	std::vector<int16_t> samples;
	load_raw_audio(is, samples);

	int16_t N = 1024;
	size_t n_samples = samples.size();
	size_t n_chunks = static_cast<size_t>(std::ceil(n_samples / double(N)))+2;	//padding may be necessary
	std::vector<int16_t> input(N*n_chunks, 0);
	std::copy(begin(samples), end(samples), begin(input) + N);

	//pre-calculate coefficients
	std::vector<double> w;
	for (int n = 0; n < 2 * N; ++n) {
		double val = std::sin((std::numbers::pi / (2 * N)) * (n + 0.5));
		w.push_back(val);
	}

	std::vector<std::vector<double>> cos;
	for (int n = 0; n < 2 * N; ++n) {
		std::vector<double> cos_n;
		for (int k = 0; k < N; ++k) {
			double val = std::cos((std::numbers::pi / N) * (n + 0.5 + N * 0.5) * (k + 0.5));
			cos_n.push_back(val);
		}
		cos.push_back(cos_n);
	}

	//calculate coefficients
	std::vector<double> coefficients;
	for (int chunk = 0; chunk < n_chunks - 1; ++chunk) {
		for (int k = 0; k < N; k++) {
			double c = 0;
			for (int n = 0; n < 2 * N; ++n) {
				c += input[chunk * N + n] * w[n] * cos[n][k];
			}
			coefficients.push_back(c);
		}
	}
	
	//step 2
	std::vector<int32_t> coefficients_quantized;
	const int32_t Q = 1000;
	for (const auto& c : coefficients) {
		coefficients_quantized.push_back(static_cast<int32_t>(c / Q));
	}

	//step 3
	double entropy1 = entropy<int32_t>(coefficients_quantized);
	

	//step 4
	std::vector<int32_t> coefficients_reconstructed;
	for (const auto& c : coefficients_quantized) {
		coefficients_reconstructed.push_back(c* Q);
	}

	//step 5
	
	
	
	
	std::ofstream os_qz("output_mdct.raw", std::ios::binary);
	if (!os_qz) {
		std::println("Error: fial opening output file {}.", "output_mdct.raw");
		return EXIT_FAILURE;
	}
	save_raw_audio(os_qz, coefficients_quantized);
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::print("Error: fail opening input file {}.", argv[1]);
		return EXIT_FAILURE;
	}
	//quantization_test(is);
	MDCT_test(is);
}

/*
	std::vector<int16_t> beg(1024);
	std::vector<int16_t> window(2048);
	for (uint32_t i = 0; i < samples.size() / 1024; i++) {
		std::vector<int16_t> end(1024);
		std::copy(samples.begin() + i * 1024, samples.begin() + (i + 1) * 1024, end);
		window.emplace_back(beg.begin(), beg.end());
		window.emplace_back(end.begin(), end.end());

		for (int k = 0; k < N; k++) {
			double c = 0;
			for (int n = 0; n < 2 * N; ++n) {
				c += window[n] * w[n] * cos[n][k];
			}
			coefficients.push_back(c);
		}

		window.clear();
		beg = end;
	}
	std::fill(window.begin(), window.end(), 0);
	window.insert(window.begin(), beg.begin(), beg.end());
	for (int k = 0; k < N; k++) {
		double c = 0;
		for (int n = 0; n < 2 * N; ++n) {
			c += window[n] * w[n] * cos[n][k];
		}
		coefficients.push_back(c);
	}
	*/
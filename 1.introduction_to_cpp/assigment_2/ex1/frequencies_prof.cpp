#include <fstream>
#include <array>
#include <iomanip>
#include <cstdint>
#include <map>
int main(int argc, char* argv[])
{
	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}
	ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	//array<size_t, 256> count{};
	map<uint8_t, size_t> count;

	char c;
	while (is.get(c)) {
		uint8_t u = c;
		count[u]+=1;
	}

	ofstream os(argv[2]/*, std::ios::binary*/);
	if (!os) {
		return EXIT_FAILURE;
	}
	for (const auto& x : count) {
		os << hex << setw(2) << setfill('0') << int(x.first)
			<< dec << '\t' << x.second<< '\n';
	}

	return EXIT_SUCCESS;
}
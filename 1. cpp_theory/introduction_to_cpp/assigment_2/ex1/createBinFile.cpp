#include <fstream>
#include <vector>

int main() {
    // Dati binari di esempio
    std::vector<unsigned char> data = { 0x01, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0xFF };

    // Scrittura su file binario
    std::ofstream output("testfile.bin", std::ios::binary);
    output.write(reinterpret_cast<const char*>(data.data()), data.size());
    output.close();

    return 0;
}
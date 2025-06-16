#ifndef HUFTSR_H
#define HUFTSR_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

struct code {
	uint8_t symb_ = 0;
	uint8_t len_= 0;
	uint32_t code_ = 0;

	code() {};
	code(uint8_t s, uint8_t l, uint32_t c) :symb_(s), len_(l), code_(c) {};
};

class hufstr {
	std::vector<code> table_;
public:
	hufstr();
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
};
#endif

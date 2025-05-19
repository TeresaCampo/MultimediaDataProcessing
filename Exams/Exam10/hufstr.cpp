#include "hufstr.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>


hufstr::hufstr() {
	/*vesion 1
	std::ifstream is("table.bin", std::ios::binary);

	std::string t_s;
	is >> t_s;
	size_t table_size = std::stol(t_s);
	is.get();

	while (table_size-- > 0) {
		uint8_t symbol;
		uint8_t len;
		uint32_t cod;

		is.read(reinterpret_cast<char*>(&symbol), 1);
		is.read(reinterpret_cast<char*>(&len), 1);
		is.read(reinterpret_cast<char*>(&cod), 4);
		table_.push_back({ symbol, len, cod } );
	}
	*/
	std::vector< code> dict = { code(32,3,0), code(101,3,1), code(97,4,4), code(105,4,5), code(108,4,6), code(110,4,7), code(111,4,8), code(114,4,9), code(99,5,20), code(100,5,21), code(115,5,22), code(116,5,23), code(117,5,24), code(39,6,50), code(44,6,51), code(103,6,52), code(109,6,53), code(112,6,54), code(118,6,55), code(9,7,112), code(10,7,113), code(13,7,114), code(46,7,115), code(91,7,116), code(93,7,117), code(98,7,118), code(102,7,119), code(104,7,120), code(34,8,242), code(49,8,243), code(83,8,244), code(113,8,245), code(122,8,246), code(50,9,494), code(51,9,495), code(58,9,496), code(59,9,497), code(65,9,498), code(68,9,499), code(69,9,500), code(71,9,501), code(73,9,502), code(48,10,1006), code(52,10,1007), code(53,10,1008), code(54,10,1009), code(55,10,1010), code(56,10,1011), code(57,10,1012), code(63,10,1013), code(67,10,1014), code(76,10,1015), code(77,10,1016), code(78,10,1017), code(80,10,1018), code(33,11,2038), code(66,11,2039), code(70,11,2040), code(79,11,2041), code(81,11,2042), code(82,11,2043), code(84,11,2044), code(45,12,4090), code(86,12,4091), code(107,12,4092), code(40,13,8186), code(41,13,8187), code(72,13,8188), code(85,13,8189), code(90,13,8190), code(75,14,16382), code(47,17,131064), code(119,18,262130), code(64,19,524262), code(74,19,524263), code(88,20,1048528), code(120,20,1048529), code(87,21,2097060), code(94,21,2097061), code(106,21,2097062), code(121,21,2097063), code(125,21,2097064), code(0,22,4194130), code(1,22,4194131), code(2,22,4194132), code(3,22,4194133), code(4,22,4194134), code(5,22,4194135), code(6,22,4194136), code(7,22,4194137), code(8,22,4194138), code(11,22,4194139), code(12,22,4194140), code(14,22,4194141), code(15,22,4194142), code(16,22,4194143), code(17,22,4194144), code(18,22,4194145), code(19,22,4194146), code(20,22,4194147), code(21,22,4194148), code(22,22,4194149), code(23,22,4194150), code(24,22,4194151), code(25,22,4194152), code(26,22,4194153), code(27,22,4194154), code(28,22,4194155), code(29,22,4194156), code(30,22,4194157), code(31,22,4194158), code(35,22,4194159), code(36,22,4194160), code(37,22,4194161), code(38,22,4194162), code(42,22,4194163), code(43,22,4194164), code(60,22,4194165), code(61,22,4194166), code(62,22,4194167), code(89,22,4194168), code(92,22,4194169), code(95,22,4194170), code(96,22,4194171), code(123,22,4194172), code(124,22,4194173), code(126,22,4194174), code(127,22,4194175), code(128,22,4194176), code(129,22,4194177), code(130,22,4194178), code(131,22,4194179), code(132,22,4194180), code(133,22,4194181), code(134,22,4194182), code(135,22,4194183), code(136,22,4194184), code(137,22,4194185), code(138,22,4194186), code(139,22,4194187), code(140,22,4194188), code(141,22,4194189), code(142,22,4194190), code(143,22,4194191), code(144,22,4194192), code(145,22,4194193), code(146,22,4194194), code(147,22,4194195), code(148,22,4194196), code(149,22,4194197), code(150,22,4194198), code(151,22,4194199), code(152,22,4194200), code(153,22,4194201), code(154,22,4194202), code(155,22,4194203), code(156,22,4194204), code(157,22,4194205), code(158,22,4194206), code(159,22,4194207), code(160,22,4194208), code(161,22,4194209), code(162,22,4194210), code(163,22,4194211), code(164,22,4194212), code(165,22,4194213), code(166,22,4194214), code(167,22,4194215), code(168,22,4194216), code(169,22,4194217), code(170,22,4194218), code(171,22,4194219), code(172,22,4194220), code(173,22,4194221), code(174,22,4194222), code(175,22,4194223), code(176,22,4194224), code(177,22,4194225), code(178,22,4194226), code(179,22,4194227), code(180,22,4194228), code(181,22,4194229), code(182,22,4194230), code(183,22,4194231), code(184,22,4194232), code(185,22,4194233), code(186,22,4194234), code(187,22,4194235), code(188,22,4194236), code(189,22,4194237), code(190,22,4194238), code(191,22,4194239), code(192,22,4194240), code(193,22,4194241), code(194,22,4194242), code(195,22,4194243), code(196,22,4194244), code(197,22,4194245), code(198,22,4194246), code(199,22,4194247), code(200,22,4194248), code(201,22,4194249), code(202,22,4194250), code(203,22,4194251), code(204,22,4194252), code(205,22,4194253), code(206,22,4194254), code(207,22,4194255), code(208,22,4194256), code(209,22,4194257), code(210,22,4194258), code(211,22,4194259), code(212,22,4194260), code(213,22,4194261), code(214,22,4194262), code(215,22,4194263), code(216,22,4194264), code(217,22,4194265), code(218,22,4194266), code(219,22,4194267), code(220,22,4194268), code(221,22,4194269), code(222,22,4194270), code(223,22,4194271), code(224,22,4194272), code(225,22,4194273), code(226,22,4194274), code(227,22,4194275), code(228,22,4194276), code(229,22,4194277), code(230,22,4194278), code(231,22,4194279), code(232,22,4194280), code(233,22,4194281), code(234,22,4194282), code(235,22,4194283), code(236,22,4194284), code(237,22,4194285), code(238,22,4194286), code(239,22,4194287), code(240,22,4194288), code(241,22,4194289), code(242,22,4194290), code(243,22,4194291), code(244,22,4194292), code(245,22,4194293), code(246,22,4194294), code(247,22,4194295), code(248,22,4194296), code(249,22,4194297), code(250,22,4194298), code(251,22,4194299), code(252,22,4194300), code(253,22,4194301), code(254,22,4194302), code(255,22,4194303)};
	table_ = dict;

}

struct bitwriter {
	std::vector<uint8_t>& ovector_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	~bitwriter() {
		//flush with ones
		flush(1);
	}

	void flush(int padding) {
		while (n_ != 0) {
			write_one_bit(padding);
		}
	}

	bitwriter(std::vector<uint8_t>& ovector) : ovector_(ovector) {};

	void write_one_bit(uint32_t val) {
		buffer_ = buffer_ << 1 | val & 1;
		n_++;

		if (n_ == 8) {
			ovector_.push_back(buffer_);
			n_ = 0;
			buffer_ = 0;
		}
	}

	void write_n_bits(uint32_t val, uint8_t nbits) {
		while (nbits--> 0) {
			write_one_bit(val >> nbits);
		}
	}
};
struct bitreader {
	const std::vector<uint8_t>& ivector_;
	size_t it = 0;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;
	bool endofvector_ = false;

	bitreader(const std::vector<uint8_t>& ivector) : ivector_(ivector) {};
	uint32_t read_one_bit() {
		if ((n_ == 0) and (it == ivector_.size())) {
			endofvector_ = true;
			return 0;
		}
		if ((n_ == 0) and it<ivector_.size()) {
			buffer_ = ivector_.at(it);
			it++;
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

	uint32_t read_n_bits( uint8_t nbits) {
		uint32_t val = 0;
		while (nbits-- > 0) {
			if (endofvector_) return 0;
			val = val << 1 | read_one_bit();
		}
	}

};
std::vector<uint8_t> hufstr::compress(const std::string& s) const {
	std::map<uint8_t, code> h_dict;
	for (const auto& e : table_) {
		h_dict[e.symb_] = e;
	}

	std::vector<uint8_t> ret;
	bitwriter bw(ret);
	for (const auto& s : s) {
		auto c = h_dict[s].code_;
		auto l = h_dict[s].len_;
		bw.write_n_bits(h_dict[s].code_, h_dict[s].len_);
	}
	bw.flush(1);

	return ret;
}

std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
	std::map<uint32_t, std::vector<code>> h_dict;
	for (const auto& e : table_) {
		h_dict[e.len_].push_back(e);
	}
	auto it = std::prev(end(h_dict));
	uint8_t max_code_len = it->first;



	std::vector<uint8_t> ret;
	bitreader br(v);
	uint32_t temp = 0;
	uint8_t len_temp = 0;
	while (!br.endofvector_) {
		temp = temp << 1 | br.read_one_bit();
		len_temp++;
		if (len_temp > max_code_len) {
			std::cout << "Error while decoding, no symbol match the encoding.\n";
			return std::string("Error while decoding, no symbol match the encoding.\n");
		}

		for (const auto& c : h_dict[len_temp]) {
			if (c.code_ == temp) {
				ret.push_back(c.symb_);

				temp = 0;
				len_temp = 0;
				break;
			}
		}
	}
	return std::string(begin(ret), end(ret));
}

/*
int main() {
	hufstr h;
	std::string test("vediamo...");

	std::vector<uint8_t> v = h.compress(test);
	h.decompress(v);

}
*/




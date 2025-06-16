#include <cstdio>
#include <print>
#include <fstream>
#include <memory>
#include <algorithm>
#include <iostream>

#include <string>
#include <map>
#include <vector>


struct node {
	size_t f_ = 0;
	uint8_t sym_ = 0;
	uint32_t code_ = 0;
	uint32_t len_ = 0;

	node* l_ = nullptr;
	node* r_ = nullptr;

	node(uint8_t sym, size_t f) : sym_(sym), f_(f) {};
	node(node* l, node* r) : l_(l), r_(r), f_(l->f_+r->f_) {};
};

struct huffman {
	std::ifstream& is_;
	std::map<uint8_t, size_t> f_;
	std::vector<node*> leaves;
	std::vector<std::unique_ptr<node>> mem_;
	
	huffman(std::ifstream& is) : is_(is) {}
	void freqency() {
		f_.clear();
		while (is_.good()) {
			f_[is_.get()] += 1;
		}
	}
	void recursive_len_update(node * n, uint32_t len) {
		if (n->l_ != nullptr) recursive_len_update(n->l_, len + 1);
		if (n->r_ != nullptr) recursive_len_update(n->r_, len + 1);
		if (n->l_ == nullptr and n->r_) { 
			n->len_ = len;
			leaves.push_back(n);
		}
	}
	void compression(std::ofstream& os) {
		is_.seekg(0, is_.end);
		size_t num_symbols = is_.tellg();
		is_.seekg(0, is_.beg);

		freqency();

		//order from most frequent(beginning) to least frequent(end)
		std::vector<node*> nodes;
		for (const auto [s, f] : f_) {
			mem_.push_back(std::make_unique<node>(s, f));
			nodes.push_back(mem_.back().get());
		}
		std::sort(begin(nodes), end(nodes), [](const node* el1, const node* el2) {
			return el1->f_ > el2->f_;
			}
		);
		while (nodes.size() > 1) {
			node* l1 = nodes.back();
			nodes.pop_back();

			node* l2 = nodes.back();
			nodes.pop_back();

			mem_.push_back(std::make_unique<node>(l1, l2));
			node* l12 = mem_.back().get();
			auto it = std::lower_bound(begin(nodes), end(nodes), l12->f_, [](const node* el1, const node* el2) {
				return el1->f_ > el2->f_;
				}
			);
			nodes.insert(it, l12);
		}
		//recursive code len update
		recursive_len_update(nodes[0], 0);

		//build codes with canonical huffman
		std::sort(begin(leaves), end(leaves), [](const node* el1, const node* el2) {
			if (el1->len_ == el2->len_) return el1->sym_ < el2->sym_;
			else return el1->len_ < el2->len_;
			}
		);
		uint32_t tmp = 0;
		uint32_t tmp_len = 0;
		for (const auto& el : leaves) {
			while (el->len_ > tmp_len) {
				tmp = tmp << 1;
				tmp_len++;
			}

			el->code_ = tmp;
			tmp++;
		}

		//print huffman compression
		std::print(os, "HUFFMAN2{}", static_cast<uint8_t>(leaves.size()));
		for (const auto& el : leaves) {

		}


	}
};

int main(int argc, char** argv) {
	if (argc != 4) {
		std::println("Error: program accepts 3 parameters, not {}", argc - 1);
		return EXIT_FAILURE;
	}

	std::string mode(argv[1]);
	if (mode != "c" and mode != "d") {
		std::println("Error: worng parameter, should be 'c' or 'd'");
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		std::println("Error: fail opening input file {}", argv[2]);
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[3], std::ios::binary);

	if (mode == "c") {
		huffman hf(is);
		hf.compression(os);
	}
}
#include <print>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <string>
#include <vector>
#include <cmath>

void print_dictionary(std::ifstream& is, int n_tabs);
void print_list(std::ifstream& is, int n_tabs);

void print_number(std::ifstream& is) {
	std::string n;
	std::getline(is, n, 'e');

	std::cout << n;
}
//returns true if 'pieces' string has just been written
bool print_string(std::ifstream& is, char token) {
	std::string n;
	std::getline(is, n, ':');

	long n_chars = std::stol(token + n);

	std::string s;
	s.resize(n_chars);
	is.read(&s[0], n_chars);
	for (size_t it = 0; it < n_chars; ++it) {
		if ((s[it] < 32) or (s[it] > 126))	s[it] = '.';
	}
	std::cout << '"' << s.data() << '"';

	if (s == "pieces") {
		return true;
	}
	else return false;
}

//returns true if 'pieces' string has just been written
bool print_one_element(std::ifstream& is, char token, int n_tabs) {
	if (token == 'i') {
		print_number(is);
		return false;
	}
	else if (std::isdigit(token)) {
		return print_string(is, token);
	}
	else if (token == 'l') {
		print_list(is, n_tabs + 1);
		return false;
	}
	else if (token == 'd') {
		print_dictionary(is, n_tabs + 1);
		return false;
	}
	else {
		std::println(std::cerr, "\n\nError: wrong file format.");
		exit(EXIT_FAILURE);
	}
}

void print_list(std::ifstream& is, int n_tabs) {
	std::cout << "[\n";
	char token = is.get();

	while (token != 'e') {
		std::cout << std::string(n_tabs + 1, '\t');

		print_one_element(is, token, n_tabs);

		std::cout << "\n";
		token = is.get();
	}
	std::cout << std::string(n_tabs, '\t');
	std::cout << "]";

}
void print_sha1(std::ifstream& is, int n_tabs, long n_blocks) {
	while (n_blocks-- > 0) {
		std::cout << '\n';

		std::string sha(20, ' ');
		is.read(sha.data(), 20);
		std::cout << std::string(n_tabs + 1, '\t');
		for (size_t it = 0; it < 20; ++it) {
			std::print("{:02x}", sha[it]);
		}
	}
}
void print_dictionary(std::ifstream& is, int n_tabs) {
	std::cout << "{\n";

	char token = is.get();
	while (token != 'e') {
		std::cout << std::string(n_tabs + 1, '\t');

		// print key
		bool flag_string = print_one_element(is, token, n_tabs);

		//print arrow
		std::cout << " => ";

		//check if we are going to read pieces value
		if (flag_string) {
			std::string n_blocks;
			std::getline(is, n_blocks, ':');

			long long_n_blocks = std::stol(n_blocks) / 20;
			print_sha1(is, n_tabs + 1, long_n_blocks);
		}
		//else print normal value
		else {
			token = is.get();
			print_one_element(is, token, n_tabs);
		}

		token = is.get();
		std::cout << '\n';
	}

	std::cout << std::string(n_tabs, '\t');
	std::cout << "}";
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::println(std::cerr,"Error: program accepts 1 parameter, {} provided.", argc - 1);
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::println(std::cerr, "Error: fail opening file {}.", argv[1]);
		return EXIT_FAILURE;
	}

	char token = is.get();
	while (!is.eof()) {
		print_one_element(is, token, -1);

		std::cout << "\n";
		token = is.get();
	}

	return EXIT_SUCCESS;
}
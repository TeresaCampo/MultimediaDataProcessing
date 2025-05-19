#include <cstdio>
#include <memory>

int main(int argc, char** argv) {
	auto p = std::make_shared<int>(7);
	std::shared_ptr<int> q = p;
}
#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <crtdbg.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <fstream>
#include <iterator>
#include <set>


//compare function for qsort
int compare(const void* a, const void* b)
{
    if (*(int*)a < *(int*)b) {
        return -1;
    }
    if (*(int*)a > *(int*)b) {
        return 1;
    }
    return 0;
}

std::vector<int> read(std::ifstream& f)
{
    using namespace std;

    vector<int> v;
    int num;

    while (f >> num) {
        v.push_back(num);
    }
    return v;
}
int main(int argc, char* argv[]) {
    using namespace std;

    if (argc != 3) {
        //std::cerr << "Usage:" << argv[0] << "<input file> <output file>\n";
        std::cerr << std::format("Usage: {} <input file> <output file>\n", argv[0]);
        return 1;
    }
    std::ifstream input(argv[1]/*, std::ios::binary*/);
    if (input.fail()) {
        std::cerr << "Error opening input file\n";
        return 1;
    }
    std::ofstream output(argv[2]/*, std::ios::binary*/);
    if (!output) {
        std::cerr << "Error opening output file\n";
        return 1;
    }
    
    std::istream_iterator<int> start(input);
    std::istream_iterator<int> stop;
    //std::set<int> v(start, stop);
    std::multiset<int> v(start, stop);



    int arr[10];
    std::copy(begin(v), end(v), std::ostream_iterator<int>(output, "\n"));

    for (const auto& x : v) {   //python form, it is called 'range based for loop'
        output << x << "\n";
    }
    return 0;
}
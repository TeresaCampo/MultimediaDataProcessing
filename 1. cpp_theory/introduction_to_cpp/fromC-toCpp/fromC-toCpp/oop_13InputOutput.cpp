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

void print(std::ofstream&  f, const std::vector<int>& v)
{
    for (const auto& x : v) {   //python form, it is called 'range based for loop'
        f << x<<"\n";
    }
}

std::vector<int> read(std::ifstream& f)
{
    using namespace std;

    vector<int> v;
    int num;

    //while (fscanf(f, "%d", &num) == 1) {
    while (f>>num) {
        v.push_back(num);
    }
    return v;
}
int main(int argc, char* argv[]) 
{
    using namespace std;

    if (argc != 3) {
        //fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        std::cerr << "Usage:" << argv[0] << "<input file> <output file>\n";
        return 1;
    }
    //* input = fopen(argv[1], "r");
    std::ifstream input(argv[1]/*, std::ios::binary*/);
    if (input.fail()) {
        std::cerr << "Error opening input file\n";
        return 1;
    }
    //FILE* output = fopen(argv[2], "w");
    std:ofstream output(argv[2]/*, std::ios::binary*/);
    if (!output) {
        std::cerr << "Error opening output file\n";
        return 1;
    }
    vector<int> v = read(input);

    std::ranges::sort(v);

    print(output, v);

    return 0;
}
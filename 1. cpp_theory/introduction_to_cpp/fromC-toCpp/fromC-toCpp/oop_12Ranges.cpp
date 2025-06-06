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

void print(FILE* f, const std::vector<int>& v)
{
    for (const auto& x : v) {   //python form, it is called 'range based for loop'
        fprintf(f, "%d\n", x);
    }
}

std::vector<int> read(FILE* f)
{
    using namespace std;

    if (f == nullptr) {
        return vector<int>();
    }

    vector<int> v;
    int num;
    while (fscanf(f, "%d", &num) == 1) {
        v.push_back(num);
    }
    return v;
}
int main(int argc, char* argv[]) {
    using namespace std;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file");
        return 1;
    }
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }
    vector<int> v = read(input);
    fclose(input);

    std::ranges::sort(v);

    print(output, v);

    fclose(output);
    return 0;
}
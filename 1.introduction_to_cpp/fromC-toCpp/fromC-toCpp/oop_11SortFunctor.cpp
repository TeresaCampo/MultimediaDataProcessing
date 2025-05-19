#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <crtdbg.h>
#include <utility>
#include <vector>
#include <algorithm>

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

//create a tou cpp object
//I create a new toy object
int global_id = 0;

struct widget {
    int id_;
    int x_;

    widget() {
        x_ = 5;
        id_ = global_id;
        global_id++;
    }
    widget(int value_x) {
        x_ = value_x;
        id_ = global_id;
        global_id++;
    }
    widget(const widget& other) {
        x_ = other.x_;
        id_ = global_id;
        global_id++;
    }

    widget& operator=(const widget& other) {
        x_ = other.x_;
        return *this;
    }
    ~widget() {
    }

    bool operator<(const widget& other) const {
        return x_ < other.x_;
    }
};

void print(FILE* f, const std::vector<int>& v)
{
    for (const auto& x : v) {   //python form, it is called 'range based for loop'
        fprintf(f, "%d\n", x);
    }
}

bool compare(int a, int b) { //it doesn't have a context
    return a > b;
}

// functor= function object
struct comparator {
    int origin_;
    //context(state information)

    bool operator()(int a, int b) { //it doesn't have a context
        return abs(long long(a-origin_)) > abs(long long(b-origin_));
    }
};

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

    long origin = 123;
    //comparator comp = { 123 };
    //comp.origin_ = 123;

    //sort(begin(v), end(v), [origin](int a, int b) { return abs(long long(a - origin)) > abs(long long(b - origin));});
    sort(begin(v), end(v), [&](int a, int b) { return abs(long long(a - origin)) > abs(long long(b - origin)); });

    fclose(output);
    return 0;
}
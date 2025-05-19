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
};

void print(FILE* f, const std::vector<int>& v)
{
    /*
    std::vector<int>::const_iterator start = v.begin();
    std::vector<int>::const_iterator stop = v.end();
    std::vector<int>::const_iterator it = start;
    int* p;
    while (it != stop) {
        int x = *it;
        fprintf(f, "%d\n", x);
        ++it;
    }
    */
    /*
    auto start = v.begin(); //auto works because compiler know datatype of v.begin()
    auto stop = v.end();
    auto it = start;
    int* p;
    while (it != stop) {
        //auto x = *it;       //like this, it is a copy constructor, akak it is doing deep copy
        const auto& x = *it;
        fprintf(f, "%d\n", x);
        ++it;
    }
    */

    for (const auto& x : v) {   //python form, it is called 'range based for loop'
        fprintf(f, "%d\n", x);
    }

}

std::vector<int> read(FILE* f)
{
    //using mdp::vector;        //import this name into this namespace
    using namespace std;        //import all the elements inside the namespace, it doesn't have to be put in global variable

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
  
    //qsort(numbers.data(), numbers.size(), sizeof(int), compare);
    std::sort(begin(v), end(v));

    fclose(output);

    return 0;
}
#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
# include <stdio.h>
# include <limits.h>
# include <stdlib.h>
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"
//# include <crtdbg.h> in visual studio
int compare_ints(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Create vector 'object'
typedef struct vector {
    int* data_;
    size_t size_, capacity_;      //size__t is 24 bytes

    // Constructor and destructor, this methods have to be called automatically by compiler
    // Default constructor
    // constructor is called automatically when object is initialized
    // example:
    //vector numbers;

    vector()
    {
        printf("Const\n");

        size_ = 0;
        capacity_ = 10;

        data_ = (int*)malloc(capacity_ * sizeof(int));
        if (data_ == NULL) {
            perror("Error allocating memory");
            exit(1);
        }
    }

    // Copy constructor, it is called when we want to initialize one vector with another
    // example:
    //vector numbers;
    // ...
    // vector original= numbers;
    vector(const vector& other) {
        printf("Copy const\n");
        size_ = other.size();
        capacity_ = other.capacity_;

        data_ = (int*)malloc(capacity_ * sizeof(int));
        for (int i = 0; i < size_; i++) {
            data_[i] = other.data_[i];
        }
    }

    // example:
    // vector numbers;
    // vector original;
    //original = numbers; <-- it is a shallow copy

    //destructor is called automatically when return is called, so if memory is allocated inside the object we don't need to write free before every return
    ~vector()
    {
        printf("Destructor\n");

        if (data_) free(data_);
    }

    // Append
    void* push_back(int value)
    {
        if (size_ == capacity_) {
            capacity_ *= 2;
            data_ = (int*)realloc(data_, capacity_ * sizeof(int));
            if (data_ == NULL) {
                perror("Error re-allocating memory");
                exit(1);
            }
        }

        data_[size_] = value;
        size_ += 1;
        return this;
    }

    // Getter for data and size_, this two methods don't modify object
    // We need to communicate this
    size_t size() const
    {
        return size_;
    }

    int at(size_t index) const
    {
        assert(index < size_);
        return data_[index];
    }

    vector& operator=(const vector& other) {
        if (capacity_ < other.size_) {
            free(data_);
            capacity_ = other.capacity_;
            data_ = (int*)malloc(capacity_ * sizeof(int));
        }

        size_ = other.size();
        for (int i = 0; i < size_; i++) {
            data_[i] = other.data_[i];
        }

        return *this;
    }
};

// This cause error because shallow copy
// this because (1)numbers --> int * (2)v.data--> int *
// SOLUTION IS USE REFERENCE, NOW v IS A POINTER TO numbers THAT THEN POINTS TO int*
void printvector_tofile(FILE* f, vector& v)
{
    for (int i = 0; i < v.size(); i++) {
        fprintf(f, "%d\n", v.at(i));
    }
}

static vector read(FILE* f) {
    // Read signed integer numbers in the file till when a non int value(neither a whitespace) is found
    int just_read;

    vector v;

    while (fscanf(f, "%d", &just_read) == 1) {
        v.push_back(just_read);
    }
    return v;           // we create a copy of the local variable and then we destruct the local variable
}


int main(int argc, char** argv) {
    {
        // Check input arguments
        char* inputf, * outputf;

        if (argc != 3) {
            fprintf(stderr, "Program accepts two parameters: input and output file (.txt format)");
            return 1;
        }
        inputf = argv[1];
        outputf = argv[2];
        printf("Correct parameters!\n-First parameter: %s\n-Second parameter: %s\n", inputf, outputf);

        // Open input file
        FILE* f = fopen(inputf, "r");
        if (f == NULL) {
            fprintf(stderr, "Error opening input file\n");
            return 1;
        }

        // Read signed integer numbers in the file till when a non int value(neither a whitespace) is found
        vector numbers;
        numbers=read(f);

        // Close input file
        fclose(f);
    
        // Sort numbers
        qsort(numbers.data_, numbers.size(), sizeof(int), compare_ints);

        // Create output file
        f = fopen(outputf, "w");
        if (f == NULL) {
            fprintf(stderr, "Error opening output file\n");
            return 1;
        }

        // Write ordered numbers in output file
        printvector_tofile(f, numbers);

        fclose(f);
    }
    // _CrtDumpMemoryLeaks(); in visual studio
    printf(GREEN "\nOutput file created successfully!" RESET);
    return 0;
}
#define _CRT_SECURE_NO_WARNINGS
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

#include <assert.h>
# include <stdio.h>
# include <limits.h>
# include <stdlib.h>


int compare_ints(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Create vector 'object'
struct vector {
    int* data;
    size_t size, capacity;      //size_t is 24 bytes
};

// Constructor and destructor
void constructor(struct vector* this) {
    this->size = 0;
    this->capacity = 10;

    this->data = malloc(this->capacity * sizeof(int));
    if (this->data == NULL) {
        perror("Error allocating memory");
        exit(1);
    }
}

void destructor(struct vector* this) {
    if (this->data) free(this->data);
}

// Append
void* push_back(struct vector* this, int value) {
    if (this->size == this->capacity) {
        this->capacity *= 2;
        this->data = realloc(this->data, this->capacity * sizeof(int));
        if (this->data == NULL) {
            perror("Error re-allocating memory");
            exit(1);
        }
    }

    this->data[this->size] = value;
    this->size += 1;
    return this;
}

//Getter for data and size
size_t size(const struct vector* this) {
    return this->size;
}

int at(const struct vector* this, size_t index) {
    assert(index >= 0 && index < this->size);
    return this->data[index];
}

int main(int argc, char** argv) {
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
    int just_read;
    int* numbers = NULL;
    int n_allocated = 0;

    while (fscanf(f, "%d", &just_read) == 1) {
        ++n_allocated;
        numbers = realloc(numbers, sizeof(int) * n_allocated);
        if (numbers) {
            numbers[n_allocated - 1] = just_read;
        }
        else {
            fprintf(stderr, "Error allocating memory\n");
            free(numbers);
            fclose(f);

            return 1;
        }
    }
    // Close input file
    fclose(f);

    // Sort numbers
    qsort(numbers, n_allocated, sizeof(int), compare_ints);
    printf("Ordered numbers:\n");
    for (int i = 0; i < n_allocated; i++) {
        printf("%d\n", numbers[i]);
    }

    // Create output file
    f = fopen(outputf, "w");
    if (f == NULL) {
        fprintf(stderr, "Error opening output file\n");
        free(numbers);
        return 1;
    }

    // Write ordered numbers in output file
    for (int i = 0; i < n_allocated; i++) {
        fprintf(f, "%d\n", numbers[i]);
    }

    fclose(f);
    free(numbers);
    printf(GREEN "\nOutput file created successfully!" RESET);
    return 0;
}
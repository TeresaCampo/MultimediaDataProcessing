#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <crtdbg.h>

//create class vector
struct vector {
	size_t size_, capacity_;
	int* data_;

	//default constructor
	//it is called when: 
	// vector v;
	vector() {
		printf("Called-> defult contructor\n");
		size_ = 0;
		capacity_ = 10;
		data_ = (int*)malloc(sizeof(int) * capacity_);
	}

	//copy construct
	//it is called when:
	// vector v_copy = v;
	// vector v_copy = vector(v)
	vector(const vector& other) {
		printf("Called-> copy constructor\n");
		size_ = other.size();
		capacity_ = other.capacity();
		data_ = (int*)malloc(sizeof(int) * capacity_);
		for (int i = 0; i < other.size(); i++) {
			data_[i] = other.at(i);
		}
	}

	~vector() {
		printf("Called-> destructor\n");
		if (data_) {
			free(data_);
		}
	}

	//assegnation
	//deep copy of r-value and return a l-value
	vector& operator=(vector& other) {
		if (capacity_ <= other.size()) {
			capacity_ *= 2;
			data_ = (int*)realloc(data_, capacity_ * sizeof(int));
		}
		size_ = other.size();
		for (int i = 0; i < other.size(); i++) {
			data_[i] = other.at(i);
		}

		return *this;
	}

	//append
	void push_back(int num) {
		if (size_ == capacity_) {
			capacity_ *= 2;
			data_ = (int*)realloc(data_, sizeof(int) * capacity_);
		}

		data_[size_] = num;
		size_ += 1;
	}

	int* data() const {
		return data_;
	}
	size_t size() const {
		return size_;
	}
	size_t capacity() const {
		return capacity_;
	}
	int at(int index) const {
		assert(index < size_);
		return data_[index];
	}
};

//compare for qsort
int compare(const void* a, const void* b) {
	int num1 = *(int*)a;
	int num2 = *(int*)b;

	if (num1 < num2) return -1;
	if (num1 > num2) return 1;
	return 0;
}

//read from file as a function
vector read_from_file(FILE* f) {
	int num;
	vector v;
	while (fscanf(f, "%d", &num) == 1) {
		v.push_back(num);
	}
	return v;
}

//function print to file
void print_to_file(FILE* f, vector& v) {
	for (int i = 0; i < v.size(); i++) {
		fprintf(f, "%d\n", v.at(i));
	}
}

int main(int argc, char** argv) {
		{
		//arguments check
		if (argc != 3) {
			printf("Program accepts exactly 2 arguments, input and output file.\n");
			return 1;
		}
		char* inputFile = argv[1];
		char* outputFile = argv[2];

		//read from file
		FILE* f = fopen(inputFile, "r");
		if (!f) {
			perror("Error opening input file.\n");
			return 1;
		}
		vector v = read_from_file(f);
		fclose(f);

		//order the vector
		qsort(v.data(), v.size(), sizeof(int), compare);

		//write in output file
		f = fopen(outputFile, "w");
		if (!f) {
			perror("Error opening/creating output file.\n");
			return 1;
		}

		//write in output file
		f = fopen(outputFile, "w");
		if (!f) {
			perror("Error opening/creating output file.\n");
			return 1;
		}
		print_to_file(f, v);
		fclose(f);

		//tests
			//create a copy
			vector v_copy1 = v;

			//multiple assegnations with int
			int x, y;
			x = 5;
			(y = x) = 7;	//debugging we can see that it is equal to y = 7
							//this because (with int) assegnations return the left value containing the new value, aka it returns a C++ reference

			//multiple assengations with vecotrs
			vector v_copy2, v_copy3;
			(v_copy2 = v_copy3) = v;	//so operator= should return a left value, aka a reference to the vector
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
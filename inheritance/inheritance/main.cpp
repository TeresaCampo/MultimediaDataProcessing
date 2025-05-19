#define _CRT_SECURE_NO_WARNINGS

#include "canvas.h"
#include <string>

#include <crtdbg.h>

struct shape { //abstract class
	int x_, y_;
	char c_;
	std::string name_;

	shape(int x, int y, char c) {
		x_ = x;
		y_ = y;
		c_ = c;
		name_ = "shape";
	}
	virtual ~shape() = default;
	void virtual draw(canvas& c) const = 0; //pure virtial method 
	
	void setname(std::string name) {
		name_ = std::move(name);
	}
};

struct point : public shape {
	int* what_; // more resources

	point(int x, int y, char c) : shape(x, y, c) {
		what_ = new int;
		what_[0] = 0x00a0ccca;
		setname("point");
	}~point() {
		delete what_;
	}
	
	void draw(canvas& c) const {
		c.set(x_, y_, c_); 
	}

};

struct line : public shape {
	int x1_, y1_;

	line(int x, int y, int x1, int y1, char c) : shape(x, y, c) {
		x1_ = x1;
		y1_ = y1;
		setname("line");
	}
	
	void draw(canvas& c) const override {
		c.line(x_, y_, x1_, y1_, c_);
	}
};

struct rectangle : public shape {
	int x1_, y1_;

	rectangle(int x, int y, int x1, int y1, char c) : shape(x, y, c) {
		x1_ = x1;
		y1_ = y1;
		setname("rectangle");
	}
	~rectangle() {
	}
	void draw(canvas& c) const {
		c.rectangle(x_, y_, x1_, y1_, c_);
	}
};

struct circle : public shape {
	int r_;

	circle(int x, int y, int r, char c) : shape(x, y, c) {
		r_ = r;
		setname("circle");
	}
	~circle() {
	}
	void draw(canvas& c) const {
		c.circle(x_, y_, r_, c_);
	}
};


int main(void)
{
	{
		canvas c(80, 25);

		rectangle* r1 = new rectangle(0, 0, 79, 24, '*');
		point* p1 = new point(5, 15, '?');
		circle* c1 = new circle(10, 10, 4, '@');
		circle* c2 = new circle(70, 10, 4, '@');
		line* l1 = new line(40, 15, 40, 20, '|');

		shape* shapes[] = { r1, p1, c1, c2, l1 };
		size_t nshapes = 5;

		for (size_t i = 0; i < nshapes; ++i) {
			shapes[i]->draw(c);
		}

		c.out(stdout);

		for (size_t i = 0; i < nshapes; ++i) {
			delete shapes[i];
		}
	}
	_CrtDumpMemoryLeaks();
}
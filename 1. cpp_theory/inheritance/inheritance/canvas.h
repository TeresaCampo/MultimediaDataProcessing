#if !defined CANVAS_H
#define CANVAS_H

#include <cstdio>

struct canvas {
    int width_, height_;
    char* ptr_;

    canvas(int width, int height);
    ~canvas();
    void set(int x, int y, char c);
    void line(int x0, int y0, int x1, int y1, char c);
    void rectangle(int x0, int y0, int x1, int y1, char c);
    void circle(int xm, int ym, int r, char c);
    void out(FILE* f);
};

#endif // CANVAS_H
#ifndef CATMULL_H
#define CATMULL_H

#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

struct Point {
    float x;
    float y;
    float z;
};

// Declaração das funções matemáticas
void buildRotMatrix(float *x, float *y, float *z, float *m);
void cross(float *a, float *b, float *res);
void normalize(float *a);
void getCatmullRomPoint(float t, Point p1, Point p2, Point p3, Point p4, float *pos, float *deriv);
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv, const std::vector<Point>& p);
void renderCatmullRomCurve(const std::vector<Point>& p);

#endif
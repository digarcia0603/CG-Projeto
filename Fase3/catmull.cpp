#include "catmull.hpp"
#include <math.h>

using namespace std;

void buildRotMatrix(float *x, float *y, float *z, float *m) {
    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void cross(float *a, float *b, float *res) {
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}

void normalize(float *a) {
    float l = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    if (l < 1e-6f) return;
    a[0] = a[0]/l; 
    a[1] = a[1]/l; 
    a[2] = a[2]/l;
}

void getCatmullRomPoint(float t, Point p1, Point p2, Point p3, Point p4, float *pos, float *deriv) {
    // Matriz de Catmull-Rom
    float m[4][4] = {{-0.5f,  1.5f, -1.5f,  0.5f},
                     { 1.0f, -2.5f,  2.0f, -0.5f},
                     {-0.5f,  0.0f,  0.5f,  0.0f},
                     { 0.0f,  1.0f,  0.0f,  0.0f}};

    float Px[4] = {p1.x, p2.x, p3.x, p4.x};
    float Py[4] = {p1.y, p2.y, p3.y, p4.y};
    float Pz[4] = {p1.z, p2.z, p3.z, p4.z};

    float T[4] = {t*t*t, t*t, t, 1};
    float Tderiv[4] = {3*t*t, 2*t, 1, 0};

    pos[0] = 0.0; pos[1] = 0.0; pos[2] = 0.0;
    deriv[0] = 0.0; deriv[1] = 0.0; deriv[2] = 0.0;

    for (int i = 0; i < 4; i++) {
        float ax = 0, ay = 0, az = 0;
        for (int j = 0; j < 4; j++) {
            ax += m[i][j] * Px[j];
            ay += m[i][j] * Py[j];
            az += m[i][j] * Pz[j];
        }
        pos[0] += T[i] * ax; pos[1] += T[i] * ay; pos[2] += T[i] * az;
        deriv[0] += Tderiv[i] * ax; deriv[1] += Tderiv[i] * ay; deriv[2] += Tderiv[i] * az;
    }
}

void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv, const vector<Point>& p) {
    int pointCount = p.size();
    float t = gt * pointCount;
    int index = floor(t);
    t = t - index;

    int p0 = (index + pointCount - 1) % pointCount;
    int p1 = (index + 0) % pointCount;
    int p2 = (index + 1) % pointCount;
    int p3 = (index + 2) % pointCount;

    getCatmullRomPoint(t, p[p0], p[p1], p[p2], p[p3], pos, deriv);
}

// Desenha a curva de Catmull-Rom no ecrã
void renderCatmullRomCurve(const vector<Point>& p) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    float pos[3], deriv[3];
    for (float gt = 0; gt < 1.0; gt += 0.01) {
        getGlobalCatmullRomPoint(gt, pos, deriv, p);
        glVertex3f(pos[0], pos[1], pos[2]);
    }
    glEnd();
}
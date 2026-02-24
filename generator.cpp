#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

void pushQuad(ofstream &file, float x1, float y1, float z1, 
                              float x2, float y2, float z2, 
                              float x3, float y3, float z3, 
                              float x4, float y4, float z4) {
    // Triângulo 1 (P1 -> P4 -> P2)
    file << x1 << " " << y1 << " " << z1 << endl;
    file << x4 << " " << y4 << " " << z4 << endl;
    file << x2 << " " << y2 << " " << z2 << endl;

    // Triângulo 2 (P2 -> P4 -> P3)
    file << x2 << " " << y2 << " " << z2 << endl;
    file << x4 << " " << y4 << " " << z4 << endl;
    file << x3 << " " << y3 << " " << z3 << endl;
}

void makePlane(float length, int divisions, string filename) {
    ofstream file(filename);
    
    // Calcular total de vértices: divisions * divisions * 2 triângulos * 3 vértices
    int numVertices = divisions * divisions * 6;
    file << numVertices << endl;

    float step = length / divisions;
    float start = -length / 2.0f;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x1 = start + i * step;
            float z1 = start + j * step;
            float x2 = start + (i + 1) * step;
            float z2 = start + (j + 1) * step;

            // Triângulo 1
            file << x1 << " 0 " << z1 << endl;
            file << x1 << " 0 " << z2 << endl;
            file << x2 << " 0 " << z2 << endl;

            // Triângulo 2
            file << x1 << " 0 " << z1 << endl;
            file << x2 << " 0 " << z2 << endl;
            file << x2 << " 0 " << z1 << endl;
        }
    }
    file.close();
}

void makeBox(float length, int divisions, string filename) {
    ofstream file(filename);
    
    // Calcular total de vértices: 6 faces * (divisions * divisions * 2 triângulos * 3 vértices)
    int numVertices = 6 * divisions * divisions * 6;
    file << numVertices << endl;
    
    float step = length / divisions;
    float start = -length / 2.0f;
    float end = length / 2.0f;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float a = start + i * step;
            float b = start + j * step;
            float next_a = start + (i + 1) * step;
            float next_b = start + (j + 1) * step;

            // Face Superior (Y fixo em +end)
            pushQuad(file, next_a, end, b,
                next_a, end, next_b,    
                a, end, next_b,         
                a, end, b);

            // Face Inferior (Y fixo em -end) - Ordem inversa para Backface Culling
            pushQuad(file,   a, -end, b, 
                           a, -end, next_b,   next_a, -end, next_b
                           , next_a, -end, b);

            // Face Frente (Z fixo em +end)
            pushQuad(file, next_a, b, end,       a, b, end, 
                a, next_b, end,       next_a, next_b, end);

            // Face Trás (Z fixo em -end)
            pushQuad(file, a, b, -end,           next_a, b, -end, 
                next_a, next_b, -end, a, next_b, -end);

            // Face Direita (X fixo em +end)
            pushQuad(file, end, next_a, b,       end, a, b, 
                end, a, next_b,       end, next_a, next_b);

            // Face Esquerda (X fixo em -end)
            pushQuad(file, -end, a, b,           -end, next_a, b, 
                -end, next_a, next_b, -end, a, next_b);
        }
    }
    file.close();
}

void makeSphere(float radius, int slices, int stacks, string filename) {
    ofstream file(filename);
    
    // Calcular total de vértices: 
    // Topo e base têm 1 triângulo por slice. O meio tem 2 triângulos por slice/stack.
    int numVertices = slices * (6 * stacks - 6);
    file << numVertices << endl;

    float alpha_step = 2 * M_PI / slices;
    float beta_step = M_PI / stacks;

    for (int i = 0; i < stacks; ++i) {
        float beta = -M_PI / 2 + i * beta_step;
        float next_beta = -M_PI / 2 + (i + 1) * beta_step;

        for (int j = 0; j < slices; ++j) {
            float alpha = j * alpha_step;
            float next_alpha = (j + 1) * alpha_step;

            // Converter coordenadas esféricas para cartesianas (x,y,z)
            float x1 = radius * cos(beta) * sin(alpha);
            float y1 = radius * sin(beta);
            float z1 = radius * cos(beta) * cos(alpha);

            float x2 = radius * cos(beta) * sin(next_alpha);
            float y2 = radius * sin(beta);
            float z2 = radius * cos(beta) * cos(next_alpha);

            float x3 = radius * cos(next_beta) * sin(next_alpha);
            float y3 = radius * sin(next_beta);
            float z3 = radius * cos(next_beta) * cos(next_alpha);

            float x4 = radius * cos(next_beta) * sin(alpha);
            float y4 = radius * sin(next_beta);
            float z4 = radius * cos(next_beta) * cos(alpha);

            // Desenhar triângulos (evitando triângulos degenerados nos polos)
            if (i != 0) { 
                file << x1 << " " << y1 << " " << z1 << endl;
                file << x2 << " " << y2 << " " << z2 << endl;
                file << x4 << " " << y4 << " " << z4 << endl;
            }
            if (i != stacks - 1) {
                file << x2 << " " << y2 << " " << z2 << endl;
                file << x3 << " " << y3 << " " << z3 << endl;
                file << x4 << " " << y4 << " " << z4 << endl;
            }
        }
    }
    file.close();
}

void makeCone(float radius, float height, int slices, int stacks, string filename) {
    ofstream file(filename);
    
    // Calcular total de vértices:
    // Base tem 1 triângulo/slice. Lateral tem (stacks-1)*2 triângulos/slice + 1 no pico.
    int numVertices = slices * 6 * stacks;
    file << numVertices << endl;
    
    float angStep = 2 * M_PI / slices;
    float heightStep = height / stacks;
    float radiusStep = radius / stacks;

    for (int i = 0; i < slices; i++) {
        float ang = i * angStep;
        float nextAng = (i + 1) * angStep;

        // 1. Base do cone (círculo no plano XZ, y=0, virado para baixo)
        file << 0.0f << " " << 0.0f << " " << 0.0f << endl;
        file << radius * sin(nextAng) << " " << 0.0f << " " << radius * cos(nextAng) << endl;
        file << radius * sin(ang) << " " << 0.0f << " " << radius * cos(ang) << endl;

        // 2. Laterais
        for (int j = 0; j < stacks; j++) {
            float y1 = j * heightStep;
            float y2 = (j + 1) * heightStep;
            
            // Raio diminui à medida que subimos
            float r1 = radius - (j * radiusStep);
            float r2 = radius - ((j + 1) * radiusStep);

            // Pontos da base atual
            float x1 = r1 * sin(ang);
            float z1 = r1 * cos(ang);
            float x2 = r1 * sin(nextAng);
            float z2 = r1 * cos(nextAng);

            // Pontos da base seguinte (mais acima e mais estreita)
            float x3 = r2 * sin(ang);
            float z3 = r2 * cos(ang);
            float x4 = r2 * sin(nextAng);
            float z4 = r2 * cos(nextAng);

            // Triângulo inferior da stack
            file << x1 << " " << y1 << " " << z1 << endl;
            file << x2 << " " << y1 << " " << z2 << endl;
            file << x3 << " " << y2 << " " << z3 << endl;

            // Triângulo superior da stack (só desenha se não for o topo)
            if (r2 > 0.0001f) { 
                file << x2 << " " << y1 << " " << z2 << endl;
                file << x4 << " " << y2 << " " << z4 << endl;
                file << x3 << " " << y2 << " " << z3 << endl;
            }
        }
    }
    file.close();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Uso: ./generator <modelo> [params...] <ficheiro_saida>" << endl;
        return 0;
    }

    if (strcmp(argv[1], "plane") == 0 && argc == 5) {
        // Ex: generator plane 1 3 plane.3d
        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        makePlane(length, divisions, argv[4]);
    }
    else if (strcmp(argv[1], "box") == 0 && argc == 5) {
        // Ex: generator box 2 3 box.3d
        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        makeBox(length, divisions, argv[4]);
    }
    else if (strcmp(argv[1], "sphere") == 0 && argc == 6) {
        // Ex: generator sphere 1 10 10 sphere.3d
        float radius = atof(argv[2]);
        int slices = atoi(argv[3]);
        int stacks = atoi(argv[4]);
        makeSphere(radius, slices, stacks, argv[5]);
    }
    else if (strcmp(argv[1], "cone") == 0 && argc == 7) {
        // Ex: generator cone 1 2 4 3 cone.3d
        float radius = atof(argv[2]);
        float height = atof(argv[3]);
        int slices = atoi(argv[4]);
        int stacks = atoi(argv[5]);
        makeCone(radius, height, slices, stacks, argv[6]);
    }
    else {
        cout << "Comando invalido ou argumentos em falta." << endl;
    }
    
    return 0;
}
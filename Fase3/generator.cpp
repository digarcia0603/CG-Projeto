#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

struct Point3D {
    float x, y, z;
};

vector<Point3D> vect;
vector<vector<int>> indices;

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

            // Face Inferior (Y fixo em -end)
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
    int numVertices = slices * (6 * stacks - 3);
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

void makeTorus(float tubeRadius, float mainRadius, int slices, int rings, string filename) {
    ofstream file(filename);
    
    // Total de vértices: slices * rings * 2 triângulos * 3 vértices
    int numVertices = slices * rings * 6;
    file << numVertices << endl;

    float uStep = 2 * M_PI / slices;
    float vStep = 2 * M_PI / rings;

    for (int i = 0; i < slices; i++) {
        float u1 = i * uStep;           // Ângulo atual à volta do eixo Y
        float u2 = (i + 1) * uStep;     // Próximo ângulo à volta do eixo Y

        for (int j = 0; j < rings; j++) {
            float v1 = j * vStep;       // Ângulo atual do tubo
            float v2 = (j + 1) * vStep; // Próximo ângulo do tubo

            // Ponto 1 (u1, v1) - Inferior Esquerdo
            float x1 = (mainRadius + tubeRadius * cos(v1)) * cos(u1);
            float y1 = tubeRadius * sin(v1);
            float z1 = (mainRadius + tubeRadius * cos(v1)) * sin(u1);

            // Ponto 2 (u2, v1) - Inferior Direito
            float x2 = (mainRadius + tubeRadius * cos(v1)) * cos(u2);
            float y2 = tubeRadius * sin(v1);
            float z2 = (mainRadius + tubeRadius * cos(v1)) * sin(u2);

            // Ponto 3 (u2, v2) - Superior Direito
            float x3 = (mainRadius + tubeRadius * cos(v2)) * cos(u2);
            float y3 = tubeRadius * sin(v2);
            float z3 = (mainRadius + tubeRadius * cos(v2)) * sin(u2);

            // Ponto 4 (u1, v2) - Superior Esquerdo
            float x4 = (mainRadius + tubeRadius * cos(v2)) * cos(u1);
            float y4 = tubeRadius * sin(v2);
            float z4 = (mainRadius + tubeRadius * cos(v2)) * sin(u1);

            
            // Triângulo 1 (P1 -> P2 -> P4)
            file << x1 << " " << y1 << " " << z1 << endl;
            file << x2 << " " << y2 << " " << z2 << endl;
            file << x4 << " " << y4 << " " << z4 << endl;

            // Triângulo 2 (P2 -> P3 -> P4)
            file << x2 << " " << y2 << " " << z2 << endl;
            file << x3 << " " << y3 << " " << z3 << endl;
            file << x4 << " " << y4 << " " << z4 << endl;
        }
    }
    file.close();
}


// calcula o valor do polinómio de Bernstein
float bernstein(int i, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;

    if (i == 0) return mt3;
    if (i == 1) return 3 * t * mt2;
    if (i == 2) return 3 * t2 * mt;
    if (i == 3) return t3;
    return 0.0f;
}

// calcula um ponto na superfície Bezier para um dado U e V
Point3D getBezierPoint(float u, float v, const vector<Point3D>& controlPoints) {
    Point3D p = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float b = bernstein(i, u) * bernstein(j, v);
            int index = i * 4 + j; // A posição na lista de 16 pontos
            p.x += controlPoints[index].x * b;
            p.y += controlPoints[index].y * b;
            p.z += controlPoints[index].z * b;
        }
    }
    return p;
}

void makePatch(string inputFile, int tessellation, string outputFile) {
    ifstream file(inputFile);
    if (!file.is_open()) {
        cout << "Erro ao abrir o ficheiro patch: " << inputFile << endl;
        return;
    }

    int numPatches;
    int numControlPoints;
    char comma;

    file >> numPatches;

    for(int i = 0; i < numPatches; i++) {
        vector<int> currentPatch;

        for (int j = 0; j < 16;j++) {
            int index;
            file >> index; // le o numero inteiro

            if (j != 15) {
                file >> comma;
            }

            currentPatch.push_back(index);
        }
        //guarda este patch de 16 indices no vetor global
        indices.push_back(currentPatch);
    }

    // le o numero de control points
    file >> numControlPoints;

    for (int i = 0; i < numControlPoints; i++) {
        float px, py, pz;
        file >> px >> comma >> py >> comma >> pz;

        Point3D p = {px, py, pz};
        vect.push_back(p);
    }

    file.close();

    //cout << "SUCESSO: Foram lidos " << indices.size() << " patches e " << vect.size() << " pontos de controlo!" << endl;

    ofstream outFile(outputFile);

    // Cada patch é dividido numa grelha (tessellation x tessellation).
    // Cada célula da grelha = 2 triângulos = 6 vértices.
    int numVertices = numPatches * tessellation * tessellation * 6;
    outFile << numVertices << endl;

    for (int p = 0; p < numPatches; p++) {
        
        // Criar uma lista só com os 16 pontos de controlo deste patch específico
        vector<Point3D> patchPoints;
        for (int i = 0; i < 16; i++) {
            int pointIndex = indices[p][i]; // O índice que lemos do ficheiro
            patchPoints.push_back(vect[pointIndex]); // O ponto real (x,y,z)
        }

        // --- 3. CRIAR A GRELHA (TESSELLATION) ---
        float step = 1.0f / tessellation;

        for (int i = 0; i < tessellation; i++) {
            for (int j = 0; j < tessellation; j++) {
                // Calcular os cantos U e V do nosso pequeno quadrado
                float u1 = i * step;
                float u2 = (i + 1) * step;
                float v1 = j * step;
                float v2 = (j + 1) * step;

                // Calcular as coordenadas X, Y, Z dos 4 cantos usando a fórmula!
                Point3D p1 = getBezierPoint(u1, v1, patchPoints);
                Point3D p2 = getBezierPoint(u2, v1, patchPoints);
                Point3D p3 = getBezierPoint(u2, v2, patchPoints);
                Point3D p4 = getBezierPoint(u1, v2, patchPoints);

                // Mandar para a tua função pushQuad para desenhar e escrever os 2 triângulos
                pushQuad(outFile, p1.x, p1.y, p1.z, 
                                  p2.x, p2.y, p2.z, 
                                  p3.x, p3.y, p3.z, 
                                  p4.x, p4.y, p4.z);
            }
        }
    }

    outFile.close();
    cout << "SUCESSO: Modelo Bezier gerado com " << numVertices << " vertices!" << endl;
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
    else if (strcmp(argv[1], "torus") == 0 && argc == 7) {
        // Ex: generator torus 0.5 2 30 30 torus.3d
        float tubeRadius = atof(argv[2]);
        float mainRadius = atof(argv[3]);
        int slices = atoi(argv[4]);
        int rings = atoi(argv[5]);
        makeTorus(tubeRadius, mainRadius, slices, rings, argv[6]);
    }
    else if (strcmp(argv[1], "patch") == 0 && argc == 5) {
        // Ex: generator patch teapot.patch 10 teapot.3d
        int tesselation = atoi(argv[3]);
        makePatch(argv[2], tesselation, argv[4]);
    }
    else {
        cout << "Comando invalido ou argumentos em falta." << endl;
    }
    
    return 0;
}
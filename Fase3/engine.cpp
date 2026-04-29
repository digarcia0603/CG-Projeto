#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "tinyxml/tinyxml2.h"
#include "catmull.hpp"

using namespace tinyxml2;
using namespace std;

// Estrutura para as Transformações Geométricas
struct Transform {
    char type; // 't' para translate, 'r' para rotate, 's' para scale
    float x, y, z, angle;

    float time;           // Tempo que demora a dar uma volta completa
    bool align;           // Se é para alinhar o modelo com a trajetória
    vector<Point> points; // Pontos de controlo para a curva de Catmull-Rom
};

// Nova estrutura para representar um modelo carregado num VBO
struct ModelVBO {
    GLuint vbo_id;    // O "id" do buffer na placa gráfica
    int vert_count;   // Quantos vértices este modelo tem
};

// Estrutura do Grupo
struct Group {
    vector<Transform> transforms;
    vector<ModelVBO> models;
    vector<Group> children; 
};

Group sceneRoot;


float camX = 0, camY = 0, camZ = 0;
float lookX = 0, lookY = 0, lookZ = 0;
float upX = 0, upY = 1, upZ = 0;
float fov = 60.0f, nearPlane = 1.0f, farPlane = 1000.0f;
int drawMode = GL_LINE;
bool showAxes = true;

int windowWidth = 512;
int windowHeight = 512;

int startX, startY, tracking = 0;
float alpha_c = 0.0f, beta_c = 0.0f, radius_c = 50.0f;



ModelVBO loadModel(const char* filename) {
    
    ModelVBO vboModel;
    vboModel.vbo_id = 0;
    vboModel.vert_count = 0;

    ifstream file(filename);
    
    if (!file.is_open()) {
        printf("Erro: Nao foi possivel abrir o ficheiro do modelo %s\n", filename);
        return vboModel;
    }

    int numVertices;
    file >> numVertices;

    // O OpenGL prefere um array gigante de floats seguido [x,y,z, x,y,z...]
    vector<float> vertexData;
    vertexData.reserve(numVertices * 3);
    
    float x, y, z;

    for (int i = 0; i < numVertices; i++) {
        file >> x >> y >> z;
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
    }
    file.close();

    GLuint buffer_id;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Guardar a referência do buffer e a quantidade de vértices
    vboModel.vbo_id = buffer_id;
    vboModel.vert_count = numVertices;

    return vboModel;
}

Group parseGroup(XMLElement* groupElement) {
    Group g;

    // 1. Ler as Transformações (se existirem)
    XMLElement* transformElement = groupElement->FirstChildElement("transform");
    if (transformElement) {
        for (XMLElement* t = transformElement->FirstChildElement(); t; t = t->NextSiblingElement()) {
            Transform tr;
            string name = t->Name();
            tr.type = name[0]; // Apanha a 1ª letra: 't', 'r' ou 's'
            
            // Inicializar variáveis a zero/false
            tr.x = 0; tr.y = 0; tr.z = 0; tr.angle = 0;
            tr.time = 0; tr.align = false;

            // Se for Translação
            if (tr.type == 't') {
                if (t->Attribute("time")) {
                    tr.time = t->FloatAttribute("time");
                    tr.align = t->BoolAttribute("align");
                    
                    // Ler os pontos de controlo da curva
                    for (XMLElement* p = t->FirstChildElement("point"); p; p = p->NextSiblingElement("point")) {
                        Point pt;
                        pt.x = p->FloatAttribute("x");
                        pt.y = p->FloatAttribute("y");
                        pt.z = p->FloatAttribute("z");
                        tr.points.push_back(pt);
                    }
                } else {
                    // Translação estática normal
                    tr.x = t->FloatAttribute("x");
                    tr.y = t->FloatAttribute("y");
                    tr.z = t->FloatAttribute("z");
                }
            } 
            // Se for Rotação
            else if (tr.type == 'r') {
                tr.x = t->FloatAttribute("x");
                tr.y = t->FloatAttribute("y");
                tr.z = t->FloatAttribute("z");
                
                if (t->Attribute("time")) {
                    tr.time = t->FloatAttribute("time");
                } else {
                    tr.angle = t->FloatAttribute("angle");
                }
            }
            // Se for Escala
            else if (tr.type == 's') {
                tr.x = t->FloatAttribute("x");
                tr.y = t->FloatAttribute("y");
                tr.z = t->FloatAttribute("z");
            }

            g.transforms.push_back(tr);
        }
    }

    // 2. Ler os Modelos .3d se existirem
    XMLElement* modelsElement = groupElement->FirstChildElement("models");
    if (modelsElement) {
        XMLElement* modelNode = modelsElement->FirstChildElement("model");
            while (modelNode != nullptr) {
                const char* file = modelNode->Attribute("file");
                if (file) {
                    // Carrega o modelo para um VBO
                    ModelVBO vbo = loadModel(file);
                    g.models.push_back(vbo);
                }
                modelNode = modelNode->NextSiblingElement("model");
            }
    }

    // 3. Ler Sub-Grupos Recursivamente
    for (XMLElement* childGroup = groupElement->FirstChildElement("group"); childGroup; childGroup = childGroup->NextSiblingElement("group")) {
        g.children.push_back(parseGroup(childGroup));
    }

    return g;
}


void loadConfig(const char* filename) {
    XMLDocument doc;
    XMLError result = doc.LoadFile(filename);

    if (result != XML_SUCCESS) {
        printf("Erro: Nao foi possivel ler o ficheiro XML %s\n", filename);
        exit(1);
    }

    XMLElement* world = doc.FirstChildElement("world");
    if (!world) return;

    // Configurar Janela
    XMLElement* window = world->FirstChildElement("window");
    if (window) {
        windowWidth = window->IntAttribute("width");
        windowHeight = window->IntAttribute("height");
    }

    // Configurar Câmara
    XMLElement* camera = world->FirstChildElement("camera");
    if (camera) {
        XMLElement* pos = camera->FirstChildElement("position");
        if (pos) {
            camX = pos->FloatAttribute("x");
            camY = pos->FloatAttribute("y");
            camZ = pos->FloatAttribute("z");
        }

        XMLElement* look = camera->FirstChildElement("lookAt");
        if (look) {
            lookX = look->FloatAttribute("x");
            lookY = look->FloatAttribute("y");
            lookZ = look->FloatAttribute("z");
        }

        XMLElement* up = camera->FirstChildElement("up");
        if (up) {
            upX = up->FloatAttribute("x");
            upY = up->FloatAttribute("y");
            upZ = up->FloatAttribute("z");
        }

        XMLElement* proj = camera->FirstChildElement("projection");
        if (proj) {
            fov = proj->FloatAttribute("fov");
            nearPlane = proj->FloatAttribute("near");
            farPlane = proj->FloatAttribute("far");
        }
    }

    // Carregar Modelos (estão dentro de <group><models><model>)
    XMLElement* worldElement = doc.FirstChildElement("world");
    if (worldElement) {
        XMLElement* currentGroup = worldElement->FirstChildElement("group");
        // Lê todos os grupos que estejam no nível principal da tag <world>
        while (currentGroup) {
            sceneRoot.children.push_back(parseGroup(currentGroup));
            currentGroup = currentGroup->NextSiblingElement("group");
        }
    }

    float dx = camX - lookX;
    float dy = camY - lookY;
    float dz = camZ - lookZ;

    radius_c = sqrt(dx * dx + dy * dy + dz * dz);

    if (radius_c > 0.0f) {
        beta_c = asin(dy / radius_c) * 180.0 / M_PI;
        alpha_c = atan2(dx, dz) * 180.0 / M_PI;
    }
}

void drawGroup(const Group& g) {
    glPushMatrix(); // Guarda o estado atual

    // Tempo decorrido em segundos desde que o programa arrancou
    float current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Aplicar Transformações do Grupo
    for (const auto& t : g.transforms) {
        if (t.type == 't') {
            if (t.time > 0 && t.points.size() >= 4) {
                // Desenhar a linha da órbita
                if (t.align == true) {
                    renderCatmullRomCurve(t.points);
                }

                // Calcular a posição animada
                float pos[3], deriv[3];
                float gt = fmod(current_time, t.time) / t.time; // Valor entre 0.0 e 1.0
                getGlobalCatmullRomPoint(gt, pos, deriv, t.points);
                glTranslatef(pos[0], pos[1], pos[2]);

                // Alinhar o modelo com a trajetória se align="true"
                if (t.align) {
                    float x[3] = {deriv[0], deriv[1], deriv[2]};
                    normalize(x);
                    float z[3];
                    float up[3] = {0.0f, 1.0f, 0.0f};
                    cross(x, up, z);
                    normalize(z);
                    float y[3];
                    cross(z, x, y);
                    normalize(y);

                    float m[16];
                    buildRotMatrix(x, y, z, m);
                    glMultMatrixf(m);
                }
            } else {
                glTranslatef(t.x, t.y, t.z);
            }
        }
        else if (t.type == 'r') {
            if (t.time > 0) {
                // Rotação animada: Dá uma volta completa (360) ao fim de "t.time" segundos
                float angle = (current_time / t.time) * 360.0f;
                glRotatef(angle, t.x, t.y, t.z);
            } else {
                glRotatef(t.angle, t.x, t.y, t.z);
            }
        }
        else if (t.type == 's') {
            glScalef(t.x, t.y, t.z);
        }
    }

   // Desenhar os modelos com VBOs
   for (const ModelVBO& model : g.models) {
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo_id);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, model.vert_count);
}

    // Desenhar os Sub-Grupos Filhos
    for (const auto& child : g.children) {
        drawGroup(child);
    }

    glPopMatrix(); // Repõe o estado para não afetar os grupos irmãos
}

void changeSize(int w, int h) {
    // Prevenir divisão por zero
    if (h == 0) h = 1;

    float ratio = w * 1.0 / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    // Usar os valores de projeção lidos do XML
    gluPerspective(fov, ratio, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
    
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, drawMode);

    // Colocar a câmara na posição lida do XML
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              upX, upY, upZ);


    // axis drawing
    if (showAxes) {
        glBegin(GL_LINES);
            // X Vermelho
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex3f(-100.0f, 0.0f, 0.0f); 
            glVertex3f(100.0f, 0.0f, 0.0f); 
            // Y Verde
            glColor3f(0.0f, 1.0f, 0.0f); 
            glVertex3f(0.0f, -100.0f, 0.0f); 
            glVertex3f(0.0f, 100.0f, 0.0f);
            // Z Azul
            glColor3f(0.0f, 0.0f, 1.0f); 
            glVertex3f(0.0f, 0.0f, -100.0f); 
            glVertex3f(0.0f, 0.0f, 100.0f);
        glEnd();
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, drawMode);

    drawGroup(sceneRoot);

    // End of frame
    glutSwapBuffers();
}

void processKeys(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            lookY += 0.5f;
            break;
        case 's':
            lookY -= 0.5f;
            break;
        case 'f': 
            drawMode = GL_FILL; 
            break;   
        case 'l': 
            drawMode = GL_LINE; 
            break;   
        case 'p': 
            drawMode = GL_POINT; 
            break;
        case 'x':
            showAxes = !showAxes;
            break;  
    }

    camX = lookX + radius_c * sin(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camZ = lookZ + radius_c * cos(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camY = lookY + radius_c * sin(beta_c * M_PI / 180.0);

    glutPostRedisplay();
}

void processSpecialKeys(int key_code, int x, int y){
    switch(key_code) {
        case GLUT_KEY_UP:
            lookZ -= 0.5f;
            break;
        case GLUT_KEY_DOWN:
            lookZ += 0.5f;
            break;
        case GLUT_KEY_LEFT:
            lookX -= 0.5f;
            break;
        case GLUT_KEY_RIGHT:
            lookX += 0.5f;
            break;
    }

    // Atualiza a posição da câmara somando a posição do alvo ao cálculo da esfera
    camX = lookX + radius_c * sin(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camZ = lookZ + radius_c * cos(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camY = lookY + radius_c * sin(beta_c * M_PI / 180.0);

    glutPostRedisplay();
}

void processMouseButtons(int button, int state, int xx, int yy) {
    if (state == GLUT_DOWN) {
        startX = xx;
        startY = yy;
        if (button == GLUT_LEFT_BUTTON)
            tracking = 1; // Rodar
        else if (button == GLUT_RIGHT_BUTTON)
            tracking = 2; // Zoom
        else
            tracking = 0;
    } 
    else if (state == GLUT_UP) {
        tracking = 0;
    }
}


void processMouseMotion(int xx, int yy) {
    if (!tracking)
        return;

    int deltaX = xx - startX;
    int deltaY = yy - startY;

    if (tracking == 1) { // Rodar
        alpha_c -= deltaX * 0.5f;
        beta_c += deltaY * 0.5f;

        if (beta_c > 85.0f) beta_c = 85.0f;
        else if (beta_c < -85.0f) beta_c = -85.0f;
    } 
    else if (tracking == 2) { // Zoom
        radius_c += deltaY * 0.5f;
        if (radius_c < 1.0f) radius_c = 1.0f;
    }

    // Reset aos pontos de partida para o próximo frame
    startX = xx;
    startY = yy;

    // Converter coordenadas esféricas para cartesianas
    camX = lookX + radius_c * sin(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camZ = lookZ + radius_c * cos(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camY = lookY + radius_c * sin(beta_c * M_PI / 180.0);

    glutPostRedisplay();
}

void idle() {
    // Diz ao GLUT para desenhar o próximo frame continuamente
    glutPostRedisplay(); 
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: ./engine config.xml\n");
        return 0;
    }

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("CG Project - Phase 3");

#ifndef __APPLE__
    glewInit(); // Inicializar o GLEW
#endif

    glEnableClientState(GL_VERTEX_ARRAY);

    // Ler a configuração XML
    loadConfig(argv[1]);

    // Required callback registry 
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    
    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
    glutIdleFunc(idle);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
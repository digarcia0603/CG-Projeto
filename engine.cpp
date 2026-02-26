#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "tinyxml/tinyxml2.h"

using namespace tinyxml2;
using namespace std;

struct Point {
    float x;
    float y;
    float z;
};


float camX = 0, camY = 0, camZ = 0;
float lookX = 0, lookY = 0, lookZ = 0;
float upX = 0, upY = 1, upZ = 0;
float fov = 60.0f, near = 1.0f, far = 1000.0f;
int drawMode = GL_LINE;

float angle = 0.0f;
float scaleY = 1.0f;

int windowWidth = 512;
int windowHeight = 512;

// Estrutura de dados para guardar todos os modelos carregados
// Um modelo é uma lista de triângulos (cada 3 pontos = 1 triângulo)
vector<vector<Point>> models;


void loadModel(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        printf("Erro: Nao foi possivel abrir o ficheiro do modelo %s\n", filename);
        return;
    }

    vector<Point> currentModel;
    
    int numVertices;
    
    if (file >> numVertices) {
        currentModel.reserve(numVertices);
    }
    
    float x, y, z;

    
    while (file >> x >> y >> z) {
        currentModel.push_back({x, y, z});
    }

    file.close();
    models.push_back(currentModel);
    printf("Modelo carregado: %s (%lu vertices)\n", filename, currentModel.size());
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
            near = proj->FloatAttribute("near");
            far = proj->FloatAttribute("far");
        }
    }

    // Carregar Modelos (estão dentro de <group><models><model>)
    XMLElement* group = world->FirstChildElement("group");
    if (group) {
        XMLElement* modelsElem = group->FirstChildElement("models");
        if (modelsElem) {
            XMLElement* model = modelsElem->FirstChildElement("model");
            // Iterar sobre todos os elementos <model>
            while (model) {
                const char* modelFile = model->Attribute("file");
                if (modelFile) {
                    loadModel(modelFile);
                }
                model = model->NextSiblingElement("model");
            }
        }
    }
}


void changeSize(int w, int h) {
    // Prevenir divisão por zero
    if (h == 0) h = 1;

    float ratio = w * 1.0 / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    // Usar os valores de projeção lidos do XML
    gluPerspective(fov, ratio, near, far);

    glMatrixMode(GL_MODELVIEW);
}


void renderScene(void) {
    
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Colocar a câmara na posição lida do XML
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              upX, upY, upZ);

    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glScalef(1.0f, scaleY, 1.0f);

    // axis drawing
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

    
    glColor3f(1.0f, 1.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, drawMode);

    // Loop para desenhar todos os modelos carregados
    for (const auto& model : models) {
        glBegin(GL_TRIANGLES);
        for (const auto& p : model) {
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }

    // End of frame
    glutSwapBuffers();
}

void processKeys(unsigned char key, int x, int y) {
    switch (key) {
        case 'a':
            angle -= 5.0f;
            break;
        case 'd':
            angle += 5.0f;
            break;
        case 'w':
            scaleY += 0.1f;
            break;
        case 's':
            scaleY -= 0.1f;
            if (scaleY < 1.0f) scaleY = 1.0f;
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
    }

    glutPostRedisplay();
}

void processSpecialKeys(int key_code, int x, int y){
    switch(key_code) {
        case GLUT_KEY_UP:
            camZ -= 0.5f;
            break;
        case GLUT_KEY_DOWN:
            camZ += 0.5f;
            break;
        case GLUT_KEY_LEFT:
            camX -= 0.5f;
            break;
        case GLUT_KEY_RIGHT:
            camX += 0.5f;
            break;
    }
    glutPostRedisplay();
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: ./engine config.xml\n");
        return 0;
    }

    // Ler a configuração XML
    loadConfig(argv[1]);

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("CG Project - Phase 1");

    // Required callback registry 
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    
    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
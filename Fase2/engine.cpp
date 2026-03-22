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

// Estrutura para as Transformações Geométricas
struct Transform {
    char type; // 't' para translate, 'r' para rotate, 's' para scale
    float x, y, z, angle;
};

// Estrutura do Grupo
struct Group {
    vector<Transform> transforms;
    vector<vector<Point>> models;
    vector<Group> children; 
};

Group sceneRoot;


float camX = 0, camY = 0, camZ = 0;
float lookX = 0, lookY = 0, lookZ = 0;
float upX = 0, upY = 1, upZ = 0;
float fov = 60.0f, near = 1.0f, far = 1000.0f;
int drawMode = GL_LINE;

float angle = 0.0f;
float scaleY = 1.0f;

int windowWidth = 512;
int windowHeight = 512;

int startX, startY, tracking = 0;
float alpha_c = 0.0f, beta_c = 0.0f, radius_c = 50.0f;



vector<Point> loadModel(const char* filename) {
    
    vector<Point> currentModel;
    ifstream file(filename);
    
    if (!file.is_open()) {
        printf("Erro: Nao foi possivel abrir o ficheiro do modelo %s\n", filename);
        return currentModel;
    }

    int numVertices;
    
    if (file >> numVertices) {
        currentModel.reserve(numVertices);
    }
    
    float x, y, z;

    while (file >> x >> y >> z) {
        currentModel.push_back({x, y, z});
    }

    file.close();
    return currentModel;
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
            
            tr.x = t->FloatAttribute("x");
            tr.y = t->FloatAttribute("y");
            tr.z = t->FloatAttribute("z");
            
            if (tr.type == 'r') {
                tr.angle = t->FloatAttribute("angle");
            } else {
                tr.angle = 0.0f;
            }
            g.transforms.push_back(tr);
        }
    }

    // 2. Ler os Modelos .3d (se existirem)
    XMLElement* modelsElement = groupElement->FirstChildElement("models");
    if (modelsElement) {
        for (XMLElement* m = modelsElement->FirstChildElement("model"); m; m = m->NextSiblingElement("model")) {
            const char* file = m->Attribute("file");
            g.models.push_back(loadModel(file));
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
            near = proj->FloatAttribute("near");
            far = proj->FloatAttribute("far");
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

void drawGroup(const Group& g) {
    glPushMatrix(); // Guarda o estado atual

    // 1. Aplicar Transformações do Grupo
    for (const auto& t : g.transforms) {
        if (t.type == 't') glTranslatef(t.x, t.y, t.z);
        else if (t.type == 'r') glRotatef(t.angle, t.x, t.y, t.z);
        else if (t.type == 's') glScalef(t.x, t.y, t.z);
    }

    // 2. Desenhar os Modelos do Grupo
    for (const auto& model : g.models) {
        glBegin(GL_TRIANGLES);
        for (const auto& p : model) {
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }

    // 3. Desenhar os Sub-Grupos Filhos
    for (const auto& child : g.children) {
        drawGroup(child);
    }

    glPopMatrix(); // Repõe o estado para não afetar os grupos "irmãos"
}

void renderScene(void) {
    
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Colocar a câmara na posição lida do XML
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              upX, upY, upZ);


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

    drawGroup(sceneRoot);

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
            camX -= 0.5f;
            break;
        case GLUT_KEY_DOWN:
            camX += 0.5f;
            break;
        case GLUT_KEY_LEFT:
            camZ -= 0.5f;
            break;
        case GLUT_KEY_RIGHT:
            camZ += 0.5f;
            break;
    }
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
    camX = radius_c * sin(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camZ = radius_c * cos(alpha_c * M_PI / 180.0) * cos(beta_c * M_PI / 180.0);
    camY = radius_c * sin(beta_c * M_PI / 180.0);

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
    glutCreateWindow("CG Project - Phase 2");

    // Required callback registry 
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    
    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glut.h>

using namespace std;

void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX = 0;
float cameraAngleY = 0;
float cameraDistance;
bool dlUsed;

vector<vector<GLdouble>> vertices;
vector<vector<int>> faces;

void loadOBJ(string filename, vector<vector<GLdouble>> &vertices, vector<vector<int>> &faces)
{
    faces.clear();
    vertices.clear();

    string line;
    char c;
    int i, j, k;
    GLdouble x, y, z;
    string si, sj, sk;

    ifstream in(filename);
    while (getline(in, line)) // read whole line
    {
        if (line.find_first_of("vVfF") == string::npos)
            continue; // skip pointless lines

        istringstream ss(line); // put line into a stream for input
        ss >> c;                // get first character
        switch (c)
        {
        case 'v':                          // VERTICES
        case 'V':                          //    (either case)
            ss >> x >> y >> z;             // read from internal stream
            vertices.push_back({x, y, z}); // add to vector
            break;

        case 'f':                 // FACES
        case 'F':                 //    (either case)
            ss >> si >> sj >> sk; // FIRST, read into individual strings
            i = stoi(si);
            j = stoi(sj);
            k = stoi(sk);                           // Get the FIRST integer from each string
            faces.push_back({i - 1, j - 1, k - 1}); // add to vector
            break;
        }
    }
    in.close();
}

void idle()
{
    glutPostRedisplay();
}

void display(void)
{
    // static int r = 0; /*rotation angle */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glLoadIdentity();

    /*Viewpoint position and line of sight direction */
    gluLookAt(20.0, 20.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    /*Rotation of figure */
    // glRotated((double)r, 0.0, 1.0, 0.0);

    // insert from teapot
    //  tramsform camera ; this helps to activate the the mouseCB and mouseMotionCB :)
    glTranslatef(0, 0, cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0); // pitch
    glRotatef(cameraAngleY, 0, 1, 0); // heading
                                      //

    // Drawing figures
    glColor3d(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);
    for (auto face : faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            glVertex3dv(&vertices[face[i]][0]);
        }
    }
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
    for (auto face : faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            glVertex3dv(&vertices[face[i]][0]);
        }
    }
    glEnd();

    glutSwapBuffers();

    /*Return the rotation angle to 0 when turning around */
    // if (++r >= 360) r = 0;
}

void resize(int w, int h)
{
    glViewport(0, 0, w, h);

    // Setting perspective transformation matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w / (double)h, 1.0, 100.0);

    // Model view transformation matrix setting
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case '\033':
        exit(0);
    default:
        break;
    }
}

void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }
}

void mouseMotionCB(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}

bool initSharedMem()
{
    mouseLeftDown = mouseRightDown = false;
    dlUsed = true;

    return true;
}

int main(int argc, char *argv[])
{
    loadOBJ("dodecahedron.obj", vertices, faces);

    initSharedMem();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(50, 50);
    glutCreateWindow(argv[0]);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glutMainLoop();

    return 0;
}

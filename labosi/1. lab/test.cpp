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

vector<vector<GLdouble>> points;
static int numPoints;
vector<vector<GLdouble>> vertices;
vector<vector<int>> faces;
vector<vector<GLdouble>> spline;

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

void loadCurve(string filename, vector<vector<GLdouble>> &points)
{
    string line;
    GLfloat x, y, z;

    ifstream in(filename);

    while (getline(in, line)) // read whole line
    {
        istringstream ss(line);      // put line into a stream for input
        ss >> x >> y >> z;           // read from internal stream
        points.push_back({x, y, z}); // add to vector
    }

    in.close();

    numPoints = points.size();

    vector<GLdouble> v0;
    vector<GLdouble> v1;
    vector<GLdouble> v2;
    vector<GLdouble> v3;

    for (int i = 0; i < numPoints - 3; i++)
    {
        v0 = points[i];
        v1 = points[i + 1];
        v2 = points[i + 2];
        v3 = points[i + 3];

        // Faktor t raste od 0 do 1
        for (int t = 0; t < 100; t++)
        {
            double j = t / 100.0;
            float f1 = (-pow(j, 3.0) + 3 * pow(j, 2.0) - 3 * j + 1) / 6.0;
            float f2 = (3 * pow(j, 3.0) - 6 * pow(j, 2.0) + 4) / 6.0;
            float f3 = (-3 * pow(j, 3.0) + 3 * pow(j, 2.0) + 3 * j + 1) / 6.0;
            float f4 = pow(j, 3.0) / 6.0;

            spline.push_back({f1 * v0[0] + f2 * v1[0] + f3 * v2[0] + f4 * v3[0],
                              f1 * v0[1] + f2 * v1[1] + f3 * v2[1] + f4 * v3[1],
                              f1 * v0[2] + f2 * v1[2] + f3 * v2[2] + f4 * v3[2]});
        }
    }
}

int t = 0;

void display(void)
{
    // static int r = 0; /*rotation angle */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glLoadIdentity();

    /*Viewpoint position and line of sight direction */
    // gluLookAt(50.0, 50.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    /*Rotation of figure */
    // glRotated((double)r, 0.0, 1.0, 0.0);

    // insert from teapot
    //  tramsform camera ; this helps to activate the the mouseCB and mouseMotionCB :)
    glTranslatef(-5.0, -5.0, -75.0);
    glRotatef(cameraAngleX, 1, 0, 0); // pitch
    glRotatef(cameraAngleY, 0, 1, 0); // heading

    glPushMatrix();

    glTranslatef(spline[t][0], spline[t][1], spline[t][2]);

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

    glPopMatrix();

    // Set the color
    glColor3f(1.0, 0.0, 0.0);
    // Set the point size
    glPointSize(3.);

    // Show the points
    glBegin(GL_POINTS);
    {
        for (int i = 0; i < numPoints; i++)
        {
            glColor3f(0.0, 1.0, 0.0);
            glVertex3dv(&points[i][0]);
        }
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (auto sp : spline)
    {
        glColor3f(1.0, 0.0, 0.0);
        glVertex3dv(&sp[0]);
    }
    glEnd();

    // Connect the points (using a piecewise linear curve)
    if (numPoints >= 2)
    {
        glColor3f(0.75, 0.75, 0.75);
        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < numPoints; i++)
            {
                glVertex3dv(&points[i][0]);
            }
        }
        glEnd();
    }

    // Force the rendering (off-screen)
    t++;
    if (t == spline.size())
        t = 0;

    glFlush();

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
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);

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

int currentTime = 0;
int previousTime = 0;

void idle()
{
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    int timeInterval = currentTime - previousTime;
    if (timeInterval > 10)
    {
        display();
        previousTime = currentTime;
    }
}

int main(int argc, char *argv[])
{
    loadOBJ("dodecahedron.obj", vertices, faces);
    loadCurve("curve.txt", points);

    initSharedMem();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("1. laboratorijska vjezba");
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);
    glutIdleFunc(idle);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glutMainLoop();

    return 0;
}

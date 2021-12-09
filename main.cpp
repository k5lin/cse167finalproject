#include <stdlib.h>
#include <iostream>
// OSX systems need their own headers
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
// Use of degrees is deprecated. Use radians for GLM functions
#define _USE_MATH_DEFINES
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include "Scene.h"

static int ttime = 0;

static const int width = 800;
static const int height = 600;
static const char* title = "Scene viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);
static Scene scene;

#include "hw3AutoScreenshots.h"

void printHelp(){
    std::cout << R"(
    Available commands:
      press 'H' to print this message again.
      press Esc to quit.
      press 'O' to save a screenshot.
      press the arrow keys to rotate camera.
      press 'A'/'Z' to zoom.
      press 'R' to reset camera.
      press 'L' to turn on/off the lighting.
    
      press Spacebar to generate images for hw3 submission.
    
)";
}

void initialize(void){
    printHelp();
    glClearColor(background[0], background[1], background[2], background[3]); // background color
    glViewport(0,0,width,height);
    
    // Initialize scene
    scene.init();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    scene.draw();
    
    glutSwapBuffers();
    glFlush();
    
}

void animation(void) {
    #define _USE_MATH_DEFINES
 
    std::map< std::string, Node* > node = scene.node;
    Node * tpot = node["teapot1"];
    glm::mat3 R = glm::mat3 (1.0f);
    
    glm::vec3 omega = { 2.25f, 0.0f, 0.1f }; 
    omega = glm::normalize(omega);

    float deltatime = glutGet(GLUT_ELAPSED_TIME); //( t - ttime ) / 1000.f;

    glm::mat3 Mmodel = glm::mat3(tpot->modeltransforms.back());
    Mmodel[0][0] = 2.25;
    Mmodel[1][1] = 2.25;
    Mmodel[2][2] = 9;

    glm::mat3 Mworld = R * Mmodel * (glm::transpose(R)); // Step 1

    glm::vec3 L = Mworld * omega; // Step 2


    for (int i = 0; i < 500; i++) { 
        

         // update omega
        omega = glm::transpose(Mworld) * L;  
        
        // Let a
        glm::vec3 a = -1.f * glm::transpose(Mworld) * (glm::cross(omega, L));

        // Let omega
        glm::vec3 newOmega = omega + (deltatime/2.f * a) + float(pow(deltatime, 2)/12.f) * glm::cross(a,omega) ;
        newOmega = glm::normalize(newOmega);

        // Update R
        glm::mat4 rotation = glm::rotate(deltatime * glm::length(newOmega), newOmega / glm::length(newOmega)); // ripped this off the writeup
        glm::mat3 Rtdt = glm::mat3(rotation) * R;
        R = Rtdt;
        glm::mat4 fin = glm::mat4(R);

        // Recompute world
        Mworld = R * Mmodel * (glm::transpose(R));

        tpot->modeltransforms.pop_back();
        tpot->modeltransforms.push_back(fin);

        glutPostRedisplay(); // redraw the object
    }

}

void saveScreenShot(const char* filename = "test.png"){
    int currentwidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentheight = glutGet(GLUT_WINDOW_HEIGHT);
    Screenshot imag = Screenshot(currentwidth,currentheight);
    imag.save(filename);
}

void keyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: // Escape to quit
            exit(0);
            break;
        case 'h': // print help
            printHelp();
            break;
        case 'o': // save screenshot
            saveScreenShot();
            break;
        case 'r':
            scene.camera -> aspect_default = float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT));
            scene.camera -> reset();
            glutPostRedisplay();
            break;
        case 'a':
            scene.camera -> zoom(0.9f);
            glutPostRedisplay();
            break;
        case 'z':
            scene.camera -> zoom(1.1f);
            glutPostRedisplay();
            break;
        case 'l':
            scene.shader -> enablelighting = !(scene.shader -> enablelighting);
            glutPostRedisplay();
            break;
        case ' ':
            hw3AutoScreenshots();
            glutPostRedisplay();
            break;
        // ! THIS WORKS ! (continuous updating)
        case 'm':
            glutIdleFunc(animation);
            break;
        case 'n':
            glutIdleFunc(NULL);
            break;
        default:
            glutPostRedisplay();
            break;
    }
}


void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP: // up
            scene.camera -> rotateUp(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: // down
            scene.camera -> rotateUp(10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT: // right
            scene.camera -> rotateRight(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT: // left
            scene.camera -> rotateRight(10.0f);
            glutPostRedisplay();
            break;
    }
}

int main(int argc, char** argv)
{
    // BEGIN CREATE WINDOW
    glutInit(&argc, argv);
    
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
#endif
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    // END CREATE WINDOW
    
    initialize();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    // put animation here
    

    glutMainLoop();
	return 0;   /* ANSI C requires main to return int. */
}

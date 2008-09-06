/*  testeOBJ.cpp
 *  aulasTP
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLUT/glut.h>
#include "glm.h"
#define PI 3.1416

// variaveis
float x=0, y=0, z=0, graus=0;
int fps=0, displayList=0;

GLfloat lightPos[4] = {50.0, 30.0 ,0.0, 0.0};
GLfloat lightAmb[3] = {0.1, 0.1, 0.1};
GLfloat lightDiff[3] = {1.0, 1.0, 1.0};

GLMmodel* f16;

void teclado(unsigned char tecla, int xx, int yy) {
	glutPostRedisplay();
	switch(tecla) {
		case 'a' : x-=2; break; // deslizar esquerda (a)
		case 'd' : x+=2; break; // deslizar direita (d)
		case 's' : z+=2; break; // deslizar tras (s)
		case 'w' : z-=2; break; // deslizar frente (w)
		case '.' : y-=2; break; // voar cima
		case ',' : y+=2; break; // voar baixo
		case 'k' : graus-=2.0; break; // olhar mais para cima (rato up)
		case 'i' : graus+=2.0; break; // olhar mais para baixo (rato down)
		case 27  : exit(0);
	}
}

void changeSize(int w, int h) {
	if(h == 0) h = 1;
	float ratio = 1.0* w / h;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glViewport(0, 0, w, h);
	gluPerspective(25, ratio, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}


void renderScene(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
	gluLookAt(15.0, 30.0,	15.0,		// Posicao da camara
			  0.0,	5.0,	0.0,		// Para onde olha
		      0.0f,	1.0f,	0.0f);		// "Angulo" da camara (0,1,0)
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
	glTranslatef(x,y,z);
	glRotatef(graus, 1.0, 0.0, 0.0);
	
	glCallList(displayList);
	
	glutSwapBuffers();
}

int main(int argc, char **argv) {
	// por inicializacao aqui
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,600);
	glutCreateWindow("F16");
	
	// por registo de funcoes aqui
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(teclado);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	
	// alguns settings para OpenLG
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	
	f16 = (GLMmodel*)malloc(sizeof(GLMmodel));
	f16 = glmReadOBJ("/Users/pittenau/Desktop/Projects/glm-example/objs/f16.obj");
	
	displayList=glGenLists(1);
	glNewList(displayList,GL_COMPILE);
		//glmList(f16, GLM_SMOOTH);
		glmDraw(f16, GLM_SMOOTH);
	glEndList();
	
	glutMainLoop();
	
	return 0;
}


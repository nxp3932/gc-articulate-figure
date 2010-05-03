#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#define WIDTH 500
#define HEIGHT 500

void SetPixel(int x, int y)
{
	glRecti(x, y, x+1, y+1);
}

void DrawStage(void)
{
	glBegin(GL_POLYGON);
		glColor3f(0.3, 0.3, 0.3);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 0, 0);
	glEnd();
}

void DrawAxis(void)
{
	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0.1, 0, 0);
	glEnd();

	glBegin(GL_LINES);
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0.1, 0);
	glEnd();

	glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 0.1);
	glEnd();
}
struct coord_t { float x; float y; float z; } ;
typedef coord_t Coord;

Coord eye = { 1.5, 1, 1.5 };
Coord lookat = { 0.5, 0, 0.5 };
Coord top = { 0, 1, 0 };

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	//gluPerspective(60.0, 1.0, 1.0, 10.0);
	//gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );
	
	glColor3f(1,1,1);
	glPushMatrix();
		DrawStage();
		DrawAxis();
	glPopMatrix();
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (WIDTH, HEIGHT); 
	glutInitWindowPosition (100, 100);

	glutCreateWindow (argv[0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1.0, 10.0);
	gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );

	DrawStage();
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glutDisplayFunc(display); 
	glutMainLoop();

	return 0;
}
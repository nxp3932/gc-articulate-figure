#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#define WIDTH 500
#define HEIGHT 500

#define MAX_JCOUNT 6

typedef struct joint_t
{
	float x, y, z, angle1, angle2; // Start
	int childCount;
	struct joint_t *children[MAX_JCOUNT], *parent;
} Joint;

Joint *addJoint(Joint *parent, float x, float y, float z, float angle1, float angle2)
{
	Joint *newJoint;
	if(!parent)
	{
		parent = (Joint *)malloc(sizeof(Joint));
		parent->parent = NULL;
	}
	else if(parent->childCount < MAX_JCOUNT)
	{
		newJoint = (Joint *)malloc(sizeof(Joint));
		newJoint->parent = parent;
		parent->children[parent->childCount++] = newJoint;
		parent = newJoint;
	}

	parent->x = x;
	parent->y = y;
	parent->z = z;
	parent->angle1 = angle1;
	parent->angle2 = angle2;
	parent->childCount = 0;

	for(int x = 0; x < MAX_JCOUNT; x++)
		parent->children[x] = NULL;

	return parent;
}

Joint *root;
void SetupFigure(void)
{
	root = addJoint(NULL, 0, 0, 0, 0, 0);
	Joint *waist = addJoint(root, 0, -0.1, 0, 0, 0);
	Joint *knee1 = addJoint(waist, -0.02, -0.05, 0, 0, 0);
	Joint *knee2 = addJoint(waist, 0.02, -0.05, 0, 0, 0);
	Joint *foot1 = addJoint(knee1, 0, -0.05, 0, 0, 0);
	Joint *foot2 = addJoint(knee2, 0, -0.05, 0, 0, 0);
	Joint *elbow1 = addJoint(root, -0.02, -0.05, 0, 0, 0);
	Joint *elbow2 = addJoint(root, 0.02, -0.05, 0, 0, 0);
	Joint *hand1 = addJoint(elbow1, 0, 0, 0.02, 0, 0);
	Joint *hand2 = addJoint(elbow2, 0, 0, 0.02, 0, 0);
}
void DrawJoint(Joint *node)
{
	glPushMatrix();

	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glColor3f(0, 1, 0);
		glVertex3f(node->x, node->y, node->z);
	glEnd();

	glTranslatef(node->x, node->y, node->z);
	for(int i = 0; i < node->childCount; i++)
	{
		DrawJoint(node->children[i]);
	}
	glPopMatrix();
	return;
}

void DrawFigure(float x, float y, float z)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	DrawJoint(root);
	glPopMatrix();
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
typedef struct coord_t { float x; float y; float z; } Coord;

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
		DrawFigure(0.5, 0.2, 0.5);
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

	SetupFigure();
	//DrawStage();
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glutDisplayFunc(display); 
	glutMainLoop();

	return 0;
}
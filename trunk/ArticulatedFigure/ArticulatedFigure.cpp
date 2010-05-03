#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#define WIDTH 500
#define HEIGHT 500

#define MAX_JCOUNT 6

bool typing = false;
int cmd_buffer_len = 0;
char cmd_buffer[100];

typedef struct joint_t
{
	char name[100];
	float x, y, z, angle1, angle2; // Start
	int childCount;
	struct joint_t *children[MAX_JCOUNT], *parent;
} Joint;

Joint *addJoint(Joint *parent, float x, float y, float z, float angle1, float angle2, char name[])
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
	strcpy(parent->name, name);

	for(int x = 0; x < MAX_JCOUNT; x++)
		parent->children[x] = NULL;

	return parent;
}

Joint *root;
Joint *selected;
void SetupFigure(void)
{
	root = addJoint(NULL, 0, 0, 0, 0, 0, "root");
	Joint *waist = addJoint(root, 0, -0.1, 0, 0, 0, "waist");
	Joint *knee1 = addJoint(waist, -0.02, -0.05, 0, 0, 0, "knee1");
	Joint *knee2 = addJoint(waist, 0.02, -0.05, 0, 0.0, 0.0, "knee2");
	Joint *foot1 = addJoint(knee1, 0, -0.05, 0, 0, 0, "foot1");
	Joint *foot2 = addJoint(knee2, 0, -0.05, 0, 0, 0, "foot2");
	Joint *elbow1 = addJoint(root, -0.02, -0.05, 0, 0, 0, "elbow1");
	Joint *elbow2 = addJoint(root, 0.02, -0.05, 0, 0, 0, "elbow1");
	Joint *hand1 = addJoint(elbow1, 0, 0, 0.02, 0, 0, "hand1");
	Joint *hand2 = addJoint(elbow2, 0, 0, 0.02, 0, 0, "hand2");
	selected = root;
}
void DrawJoint(Joint *node)
{
	glPushMatrix();
	glRotatef(node->angle1, 0.0, 0.0, 1.0);
	glRotatef(node->angle2, 1.0, 0.0, 0.0);
	glBegin(GL_LINES);
		if(node == selected)
			glColor3f(0, 1, 0);
		else
			glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
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
void DisplayCmd() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2f( 0.0, 0.0 );
	for(int i = 0; i < cmd_buffer_len; i++) {
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, cmd_buffer[i]);
	}
	glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, '_');

	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
}
typedef struct coord_t { float x; float y; float z; } Coord;

Coord eye = { 1.5, 1, 1.5 };
Coord lookat = { 0.5, 0, 0.5 };
Coord top = { 0, 1, 0 };

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glColor3f(1,1,1);
	glPushMatrix();
		DrawStage();
		DrawAxis();
		DrawFigure(0.5, 0.2, 0.5);
		if(typing)
			DisplayCmd();
	glPopMatrix();
	glutSwapBuffers();
}

void motion(int x, int y)
{
	selected->angle1 = x;
	selected->angle2 = y;
	display();
}

void selectNewLimb(Joint *node)
{
	if(strcmp(node->name, cmd_buffer) == 0)
	{
		selected = node;
	}
	else
	{
		for(int i = 0; i < node->childCount; i++)
		{
			selectNewLimb(node->children[i]);
		}
	}

}

void keyboard(unsigned char key, int x, int y)
{
	if(key == 'c' && typing == false)
	{
		typing = true;
	} else if (typing == true && key == 27) {
		cmd_buffer[0] = 0;
		cmd_buffer_len = 0;
		typing = false;
	} else if(typing == true && key == 8) {
		cmd_buffer[cmd_buffer_len--] = 0;
	} else if(typing == true && key == 13) {
		selectNewLimb(root);
		cmd_buffer[0] = 0;
		cmd_buffer_len = 0;
		typing = false;
	} else if(typing == true) {
		cmd_buffer[cmd_buffer_len++] = key;
		cmd_buffer[cmd_buffer_len] = 0;
	}
	display();
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
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
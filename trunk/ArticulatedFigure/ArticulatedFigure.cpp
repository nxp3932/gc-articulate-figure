#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <string>
#include <stdio.h>
#include <vector>
#define WIDTH 500
#define HEIGHT 500
#define FIGURE "human.txt"
#define ANIMATION "animation.txt"
#define MAX_JCOUNT 6
#define MAX_KFCOUNT 512

bool typing = false;
bool keyframe_typing = false;
int cmd_buffer_len = 0;
char cmd_buffer[100];
int frameNum = 0;
int baseTime = 0;

using namespace std;

//Define a keyframe for joints
typedef struct keyframe_t
{
	int time;
	float angle1;
	float angle2;
	struct keyframe_t *next;
} Keyframe;

typedef struct joint_t
{
    char name[100];
    float x, y, z, angle1, angle2; // Start
    int childCount;
    struct joint_t *children[MAX_JCOUNT];
	Keyframe *keyframe;
	Keyframe *curframe;
} Joint;


/* This adds a joint to a parent, or initializes it as the root if there are no parents */
Joint *addJoint(Joint *parent, float x, float y, float z, float angle1, float angle2, char name[])
{
	Joint *newJoint;
    if(!parent)
    {
        parent = (Joint *)malloc(sizeof(Joint));
    }
    else if(parent->childCount < MAX_JCOUNT)
    {
        newJoint = (Joint *)malloc(sizeof(Joint));
        parent->children[parent->childCount++] = newJoint;
        parent = newJoint;
    }

    parent->x = x;
    parent->y = y;
    parent->z = z;
    parent->angle1 = angle1;
    parent->angle2 = angle2;

	Keyframe *newframe = (Keyframe *)malloc(sizeof(Keyframe));
	newframe->angle1 = angle1;
	newframe->angle2 = angle2;
	newframe->time = 0;
	newframe->next = NULL;
	parent->curframe = newframe;
	parent->keyframe = newframe;

    parent->childCount = 0;
    strcpy(parent->name, name);

    for(int x = 0; x < MAX_JCOUNT; x++)
        parent->children[x] = NULL;

    return parent;
}

Joint *root;
Joint *selected;

Joint *FindJoint(Joint *node, char name[])
{
    if(strcmp(node->name, name) == 0)
        return node;
    else
    {
        Joint *found = NULL;
        for(int i = 0; i < node->childCount; i++)
        {
            found = FindJoint(node->children[i], name);
            if(found)
                return found;
        }
    }
    return NULL;

}

Keyframe *addKeyframe(Joint *joint, int time, float angle1, float angle2)
{
	Keyframe *newframe = (Keyframe *)malloc(sizeof(Keyframe));
	newframe->time = time;
	newframe->angle1 = angle1;
	newframe->angle2 = angle2;
	newframe->next = NULL;
	Keyframe *oldframe = joint->keyframe;
	while(oldframe->next != NULL)
		oldframe = oldframe->next;
	oldframe->next = newframe;
	if(!joint->curframe)
		joint->curframe = newframe;
	return newframe;
}

void LoadFigure(char *path)
{
    // parent x y z angle1 angle2 name
    FILE *file;
    float x=0, y=0, z=0, angle1=0, angle2=0;
    char name[20], parent[20], buffer[512];
    Joint *tmpJoint;

    if(!(file = fopen(path, "r")))
    {
        printf("Unable to load file\n");
        return;
    }

    while(!feof(file))
    {
        fgets(buffer, 512, file);
        sscanf(buffer, "%s %f %f %f %f %f %s\n", parent, &x, &y, &z, &angle1, &angle2, name);
        if(strcmp(parent, "0") == 0)
            root = addJoint(NULL, x, y, z, angle1, angle2, name);
        else
        {
            tmpJoint = FindJoint(root, parent);
            addJoint(tmpJoint, x, y, z, angle1, angle2, name);
        }
	}
	selected = root;
    fclose(file);
}

void LoadAnimation(char *path)
{
	FILE *file;
	char buffer[512], name[20];
	float angle1, angle2;
	int time;

    if(!(file = fopen(path, "r")))
    {
        printf("Unable to load file\n");
        return;
    }
	
	while(!feof(file))
    {
        fgets(buffer, 512, file);
		name[0] = 0;
        if(sscanf(buffer, "%s %f %f %d\n", name, &angle1, &angle2, &time) == 4)
		{
			Joint *framejoint = FindJoint(root, name);
			addKeyframe(framejoint, time, angle1, angle2);
		}

	}

    fclose(file);
}

void SaveTraversal(Joint *node, int time, char* buffer)
{
	printf("%f %f %d\n", node->angle1, node->angle2, time);

	sprintf(buffer+strlen(buffer), "%s %f %f %d\n", node->name, node->angle1, node->angle2, time);
	//strcat(buffer, tmpbuffer);
	for(int i = 0; i < node->childCount; i++)
    {
        SaveTraversal(node->children[i], time, buffer);
    }
}

void SaveKeyframe(char *path, int time)
{
	FILE *file;
	char buffer[1024] = "";
    if(!(file = fopen(path, "a")))
    {
        printf("Unable to load file\n");
        return;
    }
	SaveTraversal(root, time, buffer);
	fprintf(file, buffer);
	fclose(file);
}

//Animate the joint in question using interpolation
void jointAnimate(Joint *joint, int time)
{
	if(joint->curframe != NULL && joint->curframe->next != NULL && joint->curframe->next->time < time)
		joint->curframe = joint->curframe->next;
	if(joint->curframe != NULL && joint->curframe->next != NULL)
	{
		int cur_time = time - joint->curframe->time - baseTime;
		int tdelta = joint->curframe->next->time - joint->curframe->time;
		float fraction = (float)cur_time/tdelta;

		joint->angle1 = joint->curframe->angle1 + (cur_time)*(joint->curframe->next->angle1 - joint->curframe->angle1)/tdelta;
		joint->angle2 = joint->curframe->angle2 + (cur_time)*(joint->curframe->next->angle2 - joint->curframe->angle2)/tdelta;
	}
	
}

// Draws a joint and traverses to its children to draw those
void DrawJoint(Joint *node)
{
	int time = glutGet(GLUT_ELAPSED_TIME);
	jointAnimate(node, time);

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

// Draws the figure
void DrawFigure(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
	DrawJoint(root);
    glPopMatrix();
}

// Draws the stage that the figure will be located on
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

// Draws the xyz axis for reference
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

// Displays a command window to select figure joints
void DisplayCmd() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f( 0.5, 0.8 );
    for(int i = 0; i < cmd_buffer_len; i++) {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, cmd_buffer[i]);
    }
    glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, '_');

    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,1,1);
    glPushMatrix();
        DrawStage();
        DrawAxis();
        DrawFigure(0.8, 0.2, 0.6);
        if(typing)
            DisplayCmd();
    glPopMatrix();
    glutSwapBuffers();
}

typedef struct coord_t { float x; float y; float z; } Coord;
Coord eye = { 1.5, 0.8, 1.5 };
Coord lookat = { 0.5, 0, 0.5 };
Coord top = { 0, 1, 0 };
void rotateScene(float angle1, float angle2, float angle3)
{
	glTranslatef(0.5, 0, 0.5);
	glRotatef(angle3, 0, 0, 1);
	glRotatef(angle2, 1, 0, 0);
	glRotatef(angle1, 0, 1, 0);
	//gluLookAt( 0, 0, 0, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );
	glTranslatef(-0.5, 0, -0.5);
	glutPostRedisplay();
	//gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );
	return;
}

void motion(int x, int y)
{
    selected->angle1 = x;
    selected->angle2 = y;
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    if(typing == true) 
	{
		if(key == 27)
		{
			// ESC key
			cmd_buffer[0] = 0;
			cmd_buffer_len = 0;
			typing = false;
			keyframe_typing = false;
		}
		else if(key == 8 && cmd_buffer_len > 0)
		{
			// Backspace
			cmd_buffer[cmd_buffer_len--] = 0;
		}
		else if(key == 13)
		{
			// Enter key
			if(keyframe_typing)
				SaveKeyframe(ANIMATION, atoi(cmd_buffer));
			else
				selected = FindJoint(root, cmd_buffer);
			cmd_buffer[0] = 0;
			cmd_buffer_len = 0;
			typing = false;
			keyframe_typing = false;
		}
		else
		{
			// Any other key
			cmd_buffer[cmd_buffer_len++] = key;
			cmd_buffer[cmd_buffer_len] = 0;
		}
	}
	else if(key == 'j')
		rotateScene(-0.3, 0, 0);
	else if(key =='l')
		rotateScene(0.3, 0, 0);
	else if(key == 'u')
		rotateScene(0, -0.3, 0);
	else if(key == 'o')
		rotateScene(0, 0.3, 0);
	else if(key == '7')
		rotateScene(0, 0, -0.3);
	else if(key == '9')
		rotateScene(0, 0, 0.3);
	else if(key == 'r')
		LoadFigure(FIGURE);
	else if(key == 'x')
		baseTime = 	glutGet(GLUT_ELAPSED_TIME);
	else
	{
		// Enter command mode
		if(key == 'c')
			typing = true;
		else if(key == 'v')
		{
			typing = true;
			keyframe_typing = true;
		}
		else if(selected != NULL)
		{
			if(key == 'w')
				selected->angle1 += .3;
			else if(key == 's')
				selected->angle1 -= .3;
			else if(key == 'd')
				selected->angle2 += .3;
			else if(key == 'a')
				selected->angle2 -= .3;
		}
	}
	glutPostRedisplay();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (WIDTH, HEIGHT);
	glutInitWindowPosition (500, 100);

	glutCreateWindow (argv[0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1.0, 10.0);
	gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );

	LoadFigure(FIGURE);
	LoadAnimation(ANIMATION);
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glutDisplayFunc(display);
	// glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();

	return 0;
}


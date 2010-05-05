#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#define WIDTH 500
#define HEIGHT 500

#define MAX_JCOUNT 6
#define MAX_KFCOUNT 512

bool typing = false;
int cmd_buffer_len = 0;
char cmd_buffer[100];
int frameNum = 0;

//Define a keyframe for joints
typedef struct
{
	int time;
	float angle1;
	float angle2;
} Keyframe;

typedef struct joint_t
{
        char name[100];
        float x, y, z, angle1, angle2, offA1, offA2; // Start
        int childCount;
        struct joint_t *children[MAX_JCOUNT], *parent;
	int keyframeCount;
	Keyframe keyframe[MAX_KFCOUNT];
} Joint;


/* This adds a joint to a parent, or initializes it as the root if there are no parents */
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
	//parent->offA1 = 0;
	//parent->offA2 = 0;
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

void LoadFigure(char *path)
{
        // parent x y z angle1 angle2 name
        FILE *file;
        float x=0, y=0, z=0, angle1=0, angle2=0;
	int time;
        char name[20], parent[20], buffer[512], animBuf[512], *ptr, *token;
        Joint *tmpJoint;
	Keyframe *k;

        if(!(file = fopen(path, "r")))
        {
                printf("Unable to load file\n");
                return;
        }

        while(!feof(file))
        {
 		memset(animBuf, 0, 1024);
                fgets(buffer, 512, file);
                sscanf(buffer, "%s %f %f %f %f %f %s %[^\n]", parent, &x, &y, &z, &angle1, &angle2, name, animBuf);
                if(strcmp(parent, "0") == 0)
                        root = addJoint(NULL, x, y, z, angle1, angle2, name);
                else
                {
                        tmpJoint = FindJoint(root, parent);
                        addJoint(tmpJoint, x, y, z, angle1, angle2, name);
                }

		/* Now check for animation data and set up keyframes */
		if (strlen(animBuf) > 3)
		{
			ptr = animBuf;
			while ((token = strtok(ptr, " ")))
			{
				ptr = NULL;
				time = atoi(token);
			
				token = strtok(ptr, " ");
				angle1 = atof(token);

				token = strtok(ptr, " ");
				angle2 = atof(token);

				//printf("Read %d %f\n", time, angle1);

				if (tmpJoint->keyframeCount >= MAX_KFCOUNT)
				{
					printf("Can't add more keyframes\n");
					continue;
				}
				
				k = &(tmpJoint->keyframe[tmpJoint->keyframeCount]);

				k->time = time;
				k->angle1 = angle1;
				k->angle2 = angle2;
			
				tmpJoint->keyframeCount++;
			}
		}
	}
	selected = root;
        fclose(file);
}

// Draws a joint and traverses to its children to draw those
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



//Animate the joint in question using interpolation
void jointAnimate(Joint *root, int time)
 {
 	int i;
 
 	float ang1,ang2,tim;
 
 	/* Check for keyframes */
 	for (i = 0; i < root->keyframeCount; i++)			
 		if (root->keyframe[i].time == time)
 		{
 			/* Find the index for the interpolation */
 			if (i != root->keyframeCount - 1)
 			{
 				tim = root->keyframe[i + 1].time - root->keyframe[i].time;
 				ang1 = root->keyframe[i + 1].angle1 - root->keyframe[i].angle1;
 				ang2 = root->keyframe[i + 1].angle2 - root->keyframe[i].angle2;
 
 				root->offA1 = ang1 / tim;
 				root->offA2 = ang2 / tim;
  			}
 			else
 			{
 				root->offA1 = 0;
 				root->offA2 = 0;
 			}
 		}
 
 	/* Change animation */
 	root->angle1 += root->offA1;
 	root->angle2 += root->offA2;

	DrawJoint(root); 

 	/* Call on other bones */
 	for (i = 0; i < root->childCount; i++)
 		jointAnimate(root->children[i], time);
	
 }


// Draws the figure
void DrawFigure(float x, float y, float z)
{
	int i;
        glPushMatrix();
        glTranslatef(x, y, z);
	//for(i = 0; i < 10; i++)
        	jointAnimate(root , 10 );
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
typedef struct coord_t { float x; float y; float z; } Coord;

Coord eye = { 1.5, 0.5, 1.5 };
Coord lookat = { 0.5, 0, 0.5 };
Coord top = { 0, 1, 0 };

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


void motion(int x, int y)
{
        selected->angle1 = x;
        selected->angle2 = y;
        display();
}

void keyboard(unsigned char key, int x, int y)
{
        if(key == 'c' && typing == false)
                typing = true;
        else if (typing == true && key == 27) {
                cmd_buffer[0] = 0;
                cmd_buffer_len = 0;
                typing = false;
        } else if(typing == true && key == 8)
                cmd_buffer[cmd_buffer_len--] = 0;
        else if(typing == true && key == 13) {
                selected = FindJoint(root, cmd_buffer);
                cmd_buffer[0] = 0;
                cmd_buffer_len = 0;
                typing = false;
        }
        else if(typing == true)
        {
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
        glutInitWindowPosition (500, 100);

        glutCreateWindow (argv[0]);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluPerspective(60.0, 1.0, 1.0, 10.0);
        gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, top.x, top.y, top.z );

        LoadFigure("wave.txt");
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glutDisplayFunc(display);
       // glutMotionFunc(motion);
        glutKeyboardFunc(keyboard);
        glutMainLoop();

        return 0;
}


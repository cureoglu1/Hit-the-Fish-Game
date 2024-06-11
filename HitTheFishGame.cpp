/*********
----------
ADDITIONAL FEATURES: 
1. 2 different targets.
2. Fishes and Birds have different speeds.
3. Total points is kept and resetted at the end (bird: 1, fish: 2 pts.).
4. Random clouds.
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 600

#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532
#define SPEED 1

typedef struct {
	double x, y;
}point_t;

typedef struct {
	point_t pos;
}ufo_t;

typedef struct {
	bool isFired;
	point_t pos;
}laser_t;

typedef struct {
	point_t pos;
	bool active;
}target_t;
/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height
float cloudx, cloudy = 280;
float ufox = -210, ufoy = 250;

bool animate = false;
target_t fishes[5];
target_t birds[5];
laser_t laser;
int fishspawnrate = 32, birdspawnrate = 48;

int totalpoint = 0;

float remainingtime = 20.0;

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char* string, void* font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}



bool testCollisionBird(target_t t) {

	float dx = t.pos.x - laser.pos.x;
	float dy = t.pos.y - laser.pos.y;
	float d = sqrt(dx * dx + dy * dy);
	return d <= 30;
}

bool testCollisionFish(target_t t) {

	float dx = t.pos.x - laser.pos.x;
	float dy = t.pos.y - laser.pos.y;
	float d = sqrt(dx * dx + dy * dy);
	return d <= 35;
}

int findAvailableFish() {
	for (int i = 0; i < 5; i++) {
		if (fishes[i].active == false) return i;
	}
	return -1;
}
int findAvailableBird() {
	for (int i = 0; i < 5; i++) {
		if (birds[i].active == false) return i;
	}
	return -1;
}
//
// To display onto window using OpenGL commands
//
void background() {


	//gradient background
	glBegin(GL_POLYGON);

	glColor3ub(135, 206, 235);

	glVertex2f(-500, 300);
	glVertex2f(500, 300);
	glColor3ub(0, 191, 255);
	glVertex2f(500, -100);
	glVertex2f(-500, -100);
	glEnd();
	glBegin(GL_POLYGON);

	glColor3ub(0, 0, 205);


	glVertex2f(-500, -100);
	glVertex2f(500, -100);
	glColor3ub(25, 25, 112);
	glVertex2f(500, -300);
	glVertex2f(-500, -300);


	glEnd();
	glBegin(GL_LINES);
	glVertex2f(-500, -100);
	glVertex2f(500, -100);
	glEnd();
	//glColor3ub(204, 0, 0);
   // circle(0, 0, 120);

}

void scoreboard() {

	glBegin(GL_POLYGON);

	glColor4ub(0, 0, 0, 128);

	glVertex2f(325, 275);
	glVertex2f(475, 275);
	glColor4ub(105, 105, 105, 0);
	glVertex2f(475, -200);
	glVertex2f(325, -200);
	glEnd();


	glColor3f(1, 1, 1);
	vprint2(330, 260, .1, "Scoreboard");
	vprint2(330, 260, .1, "_____________");	
	vprint2(330, 240, .1, "Total Point");
	vprint2(330, 220, .1, "%d",totalpoint);
	vprint2(330, 200, .1, "Remaining Time");
	vprint2(330, 180, .1, "%2.2f",remainingtime);
	vprint2(330, 160, .1, "Keys:");
	if (animate) {
		vprint2(330, 140, .1, "<F1>: Pause/Restart");
		vprint2(330, 120, .1, "<Spacebar>: Fire");
	}
	else {
		vprint2(330, 140, .1, "<F1>: Pause/Restart");
	}


}

void cloud() {
	//clouds
	glColor3ub(220, 220, 220);
	circle(-cloudx + 120, cloudy - 70, 15);
	circle(-cloudx + 140, cloudy - 70, 20);
	circle(-cloudx + 160, cloudy - 70, 15);

	circle(-cloudx + 20, cloudy - 40, 15);
	circle(-cloudx, cloudy - 40, 20);
	circle(-cloudx - 20, cloudy - 40, 15);

	circle(cloudx - 160, cloudy - 50, 15);
	circle(cloudx - 140, cloudy - 50, 20);
	circle(cloudx - 120, cloudy - 50, 15);

	circle(cloudx - 280, cloudy - 30, 15);
	circle(cloudx - 260, cloudy - 30, 20);
	circle(cloudx - 240, cloudy - 30, 15);

	circle(cloudx - 20, cloudy - 70, 15);
	circle(cloudx, cloudy - 70, 20);
	circle(cloudx + 20, cloudy - 70, 15);


}

void draw_laser() {

	if (laser.isFired) {

		glLineWidth(5);
		glColor3f(0, 0, 0);

		glBegin(GL_LINES);
		glVertex2f(laser.pos.x, laser.pos.y - 16);
		glVertex2f(laser.pos.x, laser.pos.y + 16);
		glEnd();

		glLineWidth(3);
		glColor3f(0, 1, 0);

		glBegin(GL_LINES);
		glVertex2f(laser.pos.x, laser.pos.y - 15);
		glVertex2f(laser.pos.x, laser.pos.y + 15);
		glEnd();
		glLineWidth(1);



	}



}


void ufo() {
	glColor3f(0, 0, 0);
	circle(ufox, ufoy, 26);
	glColor3ub(152, 251, 152);
	circle(ufox, ufoy, 25);
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 0);
	glVertex2f(ufox - 26, ufoy);
	glVertex2f(ufox + 26, ufoy);
	glVertex2f(ufox + 52, ufoy - 36);
	glVertex2f(ufox - 52, ufoy - 36);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3ub(96, 96, 96);
	glVertex2f(ufox - 25, ufoy);
	glVertex2f(ufox + 25, ufoy);
	glVertex2f(ufox + 50, ufoy - 35);
	glVertex2f(ufox - 50, ufoy - 35);
	glEnd();

	glColor3f(1, 1, 0);
	circle(ufox - 30, ufoy - 15, 3);
	circle(ufox - 10, ufoy - 15, 3);
	circle(ufox + 10, ufoy - 15, 3);
	circle(ufox + 30, ufoy - 15, 3);

	/* glColor3f(1, 1, 0);
	circle(ufox - 15, ufoy - 15, 3);
	circle(ufox + 5, ufoy - 15, 3);
	circle(ufox + 25, ufoy - 15, 3);*/
}

void bird(target_t* bird) {

	if (!bird->active) return;


	glColor3ub(255, 215, 0);
	circle(bird->pos.x + 10, bird->pos.y - 10, 20);

	circle(bird->pos.x + 15, bird->pos.y - 10, 20);
	glColor3f(1, 1, 1);
	circle(bird->pos.x, bird->pos.y, 10);
	glColor3f(0, 0, 0);
	circle(bird->pos.x - 5, bird->pos.y, 3);

	/*circle(bird->pos.x+20, bird->pos.y, 10);*/
	glBegin(GL_POLYGON);
	glColor3ub(254, 152, 0);
	glVertex2f(bird->pos.x - 15, bird->pos.y - 10);
	glVertex2f(bird->pos.x + 15, bird->pos.y - 10);
	glVertex2f(bird->pos.x + 20, bird->pos.y - 20);
	glVertex2f(bird->pos.x - 15, bird->pos.y - 15);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3ub(236, 150, 20);
	glVertex2f(bird->pos.x + 15, bird->pos.y + 5);
	glVertex2f(bird->pos.x + 35, bird->pos.y + 10);
	glVertex2f(bird->pos.x + 40, bird->pos.y - 15);
	glVertex2f(bird->pos.x + 15, bird->pos.y - 10);
	glEnd();

}


void fish(target_t* fish) {

	if (!fish->active) return;


	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0);
	glVertex2f(fish->pos.x + 15, fish->pos.y + 5);
	glVertex2f(fish->pos.x + 35, fish->pos.y + 10);
	glVertex2f(fish->pos.x + 40, fish->pos.y - 15);
	glVertex2f(fish->pos.x + 15, fish->pos.y - 10);
	glEnd();

	/*glColor3f(1, 1, 0);
	circle(fish->pos.x+31, fish->pos.y, 10);
	*/

	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1);
	glVertex2f(fish->pos.x - 10, fish->pos.y);
	glVertex2f(fish->pos.x + 30, fish->pos.y + 35);
	glVertex2f(fish->pos.x + 30, fish->pos.y - 35);
	glEnd();

	glColor3f(1, 1, 1);
	circle(fish->pos.x + 7, fish->pos.y + 10, 5);
	glColor3f(0, 0, 0);
	circle(fish->pos.x + 5, fish->pos.y + 10, 3);

	glBegin(GL_POLYGON);
	glColor3f(0, 0, 0);
	glVertex2f(fish->pos.x - 5, fish->pos.y - 5);
	glVertex2f(fish->pos.x + 10, fish->pos.y - 5);
	glVertex2f(fish->pos.x + 10, fish->pos.y - 10);
	glVertex2f(fish->pos.x, fish->pos.y - 10);
	glEnd();

}

void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	background();



	ufo();
	cloud();
	for (int i = 0; i < 5; i++)
		fish(&fishes[i]);
	for (int i = 0; i < 5; i++)
		bird(&birds[i]);


	draw_laser();

	scoreboard();

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)

{
	if (animate && !laser.isFired &&key == ' ') {

		laser.pos = { ufox,ufoy };
		laser.isFired = true;

}
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP:
		up = true;
		break;
	case GLUT_KEY_DOWN:
		down = true;
		break;
	case GLUT_KEY_LEFT:
		left = true;
		break;
	case GLUT_KEY_RIGHT:
		right = true;
		break;
	case GLUT_KEY_F1:
		animate = !animate;
		if (remainingtime == 0) {
			remainingtime = 20;
			for (int i = 0; i < 5; i++) {
				birds[i].active = false;
			}
			birdspawnrate = 48;
			for (int i = 0; i < 5; i++) {
					fishes[i].active = false;
			}
			fishspawnrate = 32;
			totalpoint = 0;
		}
		break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP:
		up = false;
		break;
	case GLUT_KEY_DOWN:
		down = false;
		break;
	case GLUT_KEY_LEFT:
		left = false;
		break;
	case GLUT_KEY_RIGHT:
		right = false;
		break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	

	if (animate) {
		
		if (remainingtime > 0)remainingtime -= (1 / 60.0);
		else {
			remainingtime = 0.0;
			animate = false;
		}

		
		if (right) {
		ufox += 3;
	}
	if (left) {
		ufox -= 3;
	}

	if (fishspawnrate == 0) {
		int availFish = findAvailableFish();
		if (availFish != -1) {
			fishes[availFish].pos.x = 500;
			fishes[availFish].pos.y = -(rand() % 150 + 150);
			fishes[availFish].active = true;
			fishspawnrate = 32;
		}
	}

	if (fishspawnrate > 0) fishspawnrate--;

	if (birdspawnrate == 0) {
		int availBird = findAvailableBird();
		if (availBird != -1) {
			birds[availBird].pos.x = 500;
			birds[availBird].pos.y = (rand() % 50 + 1);

			
			birds[availBird].active = true;
			birdspawnrate = 48;
		}
	}

	if (birdspawnrate > 0) birdspawnrate--;

		for (int i = 0; i < 5; i++) {
			if (birds[i].pos.x<475 && birds[i].pos.x>-475) {
				birds[i].pos.x -= SPEED;
			}
			else {
				birds[i].pos.x = 474;
			}
		}


		for (int i = 0; i < 5; i++) {
			if (fishes[i].pos.x<475 && fishes[i].pos.x>-475) {
				fishes[i].pos.x -= SPEED * 2;
			}
			else {
				fishes[i].pos.x = 474;
			}
		}


		if (laser.isFired && laser.pos.y>-285) {
			laser.pos.y -= 20;
		}
		else {
			laser.isFired = false;
		}

		for (int i = 0; i < 5; i++) {
			if (fishes[i].active && laser.isFired && testCollisionFish(fishes[i])) {
				fishes[i].active = false;
				totalpoint += 2;
			}
		}
		for (int i = 0; i < 5; i++) {
			if (birds[i].active &&laser.isFired&& testCollisionBird(birds[i])) {
				birds[i].active = false;
				totalpoint++;
			}
		}



	}
	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}



void main(int argc, char* argv[]) {
	srand(time(NULL));

	cloudx = rand() % 350 + 150;




	
	//find available fish | find available bird
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Ufo loots from Mustafa Cureoglu's Earth");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}
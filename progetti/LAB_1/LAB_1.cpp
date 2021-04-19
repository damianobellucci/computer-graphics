/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */

/* PUNTI DA SODDISFARE:
1,OK) provare i controlli da keyboard. tasto sinistro si aggiunge un punto,
i comandi 'f' (lettera effe) e 'l' (lettera elle) rimuovono il primo e l'ultimo 
punto della lista di punti, rispettivamente. Oltre i 64 punti, i primi verranno 
rimossi.
2,OK) osservare come il programma usa le OpenGL GLUT callback per catturare
gli eventi click del mouse e determinare le posizioni (x,y) relative
3,OK) utilizzare i punti di controllo inseriti, utilizzando l'algoritmo di de
Casteljau
*/


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

/*questo valore ci serve per settare un limite dei punti di controllo
disegnabili oltre la quale se si disegna un nuovo punto verrà eliminato
un'altro all'inizio
*/
#define MaxNumPts 120


static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

using namespace glm;

/*5 point variables*/
int mouseOverIndex = -1;
bool isMovingPoint = false;
int movingPoint = -1;


float PointArray[MaxNumPts][3];

float CurveArray[MaxNumPts][3];

int NumPts = 0;

int i, j;

// Window size in pixels 
int		width = 500;
int		height = 500;

/* Prototypes */
void addNewPoint(float x, float y);
void removeFirstPoint();
void removeLastPoint();

  
/*
Funzione che data in input alla glutKeyboardFunc 

permmette di cancellare primo punto di controllo con f e ultimo punto
di controllo con l
*/
void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}

/*
funzione usata in myKeyboardFunc (se viene premuto tasto f)
e in funzione addNewPoint se numero massimo di punti viene
ecceduto  
*/
void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];
			PointArray[i][2] = 0;
		}
	}
}

/*
funzione usata se viene premuto il tasto l (elle) oppure
se viene sorprassato il numero massimo di punti ammissibili
*/
void removeLastPoint() {
	if (NumPts > 0) {
		NumPts--;
	}
}

void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

/*
Funzione da dare in input alla glutMouseFunc 

serve per controllare i comandi di spostamento punti di controllo
tramite mouse
*/
void myMouseFunc(int button, int state, int x, int y) {

	/*condizione per controllare che tasto sinistro sia stato premuto e
	mantenuto giù
	con GLUT_LEFT_BUTTON controllo che sia stato premuto e con glut down
	che sia mantenuto giù
	*/
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (mouseOverIndex != -1) {
			isMovingPoint = true;
			movingPoint = mouseOverIndex;
			return;
		}
		// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		printf("new point Xpos %f Ypos %f \n", xPos, yPos);
		printf("new pixel x %d y %d \n", x, y);
		addNewPoint(xPos, yPos);
		glutPostRedisplay();
	}
}

/*
Funzione da dare in input alla glutPassiveMotionFunc

Controlla gli spostamenti passivi del mouse, cioè quelli che avvengono
senza che venga toccato un altro comando. Il compito di questa funzione
è quello di attivare il flag mouse over index se si rileva che la distanza
del puntatore ed uno dei punti di controllo disegnati è sotto una certa soglia, 
cioè il puntatore è approssimabilmente sopra il punto.
*/
void myPassiveMotionFunction(int x, int y) {
	float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
	float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

	for (int i = 0; i < NumPts; i++) {
		float dist = sqrt(pow(PointArray[i][0] - xPos, 2) + pow(PointArray[i][1] - yPos, 2));
		if (dist < 0.03) {
			mouseOverIndex = i;
			glutPostRedisplay();
			return;
		}
		else {
			mouseOverIndex = -1;
		}
	}
	glutPostRedisplay();
}

/*
Funzione da dare in input alla glutMotionFunc

permette di aggiornare le posizioni de punti di controllo
mentre vengono spostati
*/
void myMotionFunction(int x, int y) {
	float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
	float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

	if (isMovingPoint) {
		PointArray[movingPoint][0] = xPos;
		PointArray[movingPoint][1] = yPos;
	}
	glutPostRedisplay();
}

/* 
Funzione che permette di aggiungere un nuovo punto
alla fine della lista. Si rimuove il primo punto
nella lista se ci sono troppi punti.
*/ 
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}
	PointArray[NumPts][0] = x;
	PointArray[NumPts][1] = y;
	PointArray[NumPts][2] = 0;
	NumPts++;
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}


/*
nella init vanno tutte le cose che si fanno una volta sola, prima di renderizare
*/
void init(void)
{

	//control polygon data
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//curve points data
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, width, height);

}

void deCasteljau(float t, float* result) {
	int i, k;
	float coordX[MaxNumPts], coordY[MaxNumPts];

	for (i = 0; i < NumPts; i++) {
		coordX[i] = PointArray[i][0];
		coordY[i] = PointArray[i][1];
	}

	for (i = 1; i < NumPts; i++) {
		for (k = 0; k < NumPts - i; k++) {
			coordX[k] = (1 - t) * coordX[k] + (t)*coordX[k + 1];
			coordY[k] = (1 - t) * coordY[k] + (t)*coordY[k + 1];
		}
	}
	result[0] = coordX[0];
	result[1] = coordY[0];
	result[2] = 0.0;
}

/*
Questa funzione va in input alla funzione glut glutDisplayFunc

è la funzione che effettivamente fa la renderizzazione.

Come fa la drawArrays a sapere dove deve disegnare i punti?


*/

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Draw the line segments
	glLineWidth(2.5);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);


	// Draw the points
	glPointSize(8.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	glBindVertexArray(0);

	if (NumPts > 1) {
		float result[3];

	
		for (i = 0; i <= 100; i++) {
			deCasteljau((GLfloat)i / 100, result);
			CurveArray[i][0] = result[0];
			CurveArray[i][1] = result[1];    
			CurveArray[i][2] = 0;
		}
		
		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glLineWidth(0.5);
		glDrawArrays(GL_LINE_STRIP, 0, 101);
		glBindVertexArray(0);
	}   
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);

	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(myMotionFunction);
	glutPassiveMotionFunc(myPassiveMotionFunction);

	



	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}

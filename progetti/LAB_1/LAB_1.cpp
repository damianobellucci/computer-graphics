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

 3,OK) Disegnare la curva di Bezier a partire dai punti di controllo inseriti,
 utilizzando l'algoritmo di de Casteljau

 4, DA FARE) disegno di una curva di bezier mediante algoritmo ottimizzato,
 basato sulla suddivisione adattiva


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
#define nMaxPoints 120

int chosenModality;

static unsigned int programId;

unsigned int VAO_CONTROL_POLYGON;
unsigned int VBO_CONTROL_POLYGON;

unsigned int VAO_CURVE_POINTS;
unsigned int VBO_CURVE_POINTS;

using namespace glm;

/*5 point variables*/
int mouseOverIndex = -1;
bool isMovingPoint = false;
int movingPoint = -1;

float pointsInWindow[nMaxPoints][3];

float pointsCurve[nMaxPoints][3];

// Window size in pixels
int width = 500;
int height = 500;

int nCurrentPoints = 0;
int NumPtsS = 0;

float points[nMaxPoints][3];
float subdivisionFactor = 0.5;

//parametro di precisione suddivisione adattiva
float precisionPlanarity;

int i, j, xy;
float x = 0, y = 0;


/* Prototypes */
void addNewPoint(float x, float y);
void removeFirstPoint();
void removeLastPoint();

// Calcolo la distanza di un punto da un segmento
float distToLine(vec3 pt1, vec3 pt2, vec3 point)
{
    vec2 lineDir = pt2 - pt1;
    vec2 perpDir = vec2(lineDir.y, -lineDir.x);
    vec2 dirToPt1 = pt1 - point;
    return abs(dot(normalize(perpDir), dirToPt1));
}

//algoritmo di suddivisione adattiva
void adaptiveSubdivision(float points[nMaxPoints][3], int nPoints)
{
    bool continueRecursion = false;

    vec3 firstPoint;
    firstPoint[0] = points[0][0];
    firstPoint[1] = points[0][1];
    firstPoint[2] = 0;

    vec3 lastPoint;
    lastPoint[0] = points[nPoints - 1][0];
    lastPoint[1] = points[nPoints - 1][1];
    lastPoint[2] = 0;

    //test di planarità sui control point interni. parto dal secondo (perché il primo è il primo estremo della retta per il test di planarità)
    for (int i = 1; i < nPoints - 1; i++)
    {
        vec3 nextPoint;
        nextPoint[0] = points[i][0];
        nextPoint[1] = points[i][1];
        nextPoint[2] = 0;

        //eseguo il test di planarità
        float dist = distToLine(firstPoint, lastPoint, nextPoint);

        //se distanza tra punto e retta è maggiore della tolleranza scelta, allora devo continuare a suddividere perché l'interpolazione non è abbastanza precisa secondo i parametri
        if (dist > precisionPlanarity)
            continueRecursion = true;
    }
    if (continueRecursion) {
        {
            float firstHalfCurve[nMaxPoints][3];
            float secondHalfCurve[nMaxPoints][3];

            //carico i nuovi punti
            for (int i = 0; i < nPoints; i++)
            {
                firstHalfCurve[i][0] = points[0][0];
                firstHalfCurve[i][1] = points[0][1];
                secondHalfCurve[nPoints- i - 1][0] = points[nPoints - i - 1][0];
                secondHalfCurve[nPoints - i - 1][1] = points[nPoints - i - 1][1];

                //applicazione decasteljau per le due sottocurve per calcolare i punti intermedi di ogni sottocurva
                for (int j = 0; j < 2; j++)
                    for (int k = 0; k < nPoints-1; k++) //se i punti sono 3 si fa solo 0 ed 1
                    {
                        //calcolo della lerp
                        points[k][j] = points[k][j] * (1 - subdivisionFactor) + points[k + 1][j] * subdivisionFactor;
                    }
            }
            //vado in ricorsione sulle due sottocurve ottenute dividendo quella iniziale

            //passo le due sottocurve in ricorsione passando anche la dimensione delle due sottocurve. Siccome le due sottocurve ottenute
            //hanno lo stesso numero di punti di quella di partenza, si passa sempre npoints. prima o poi si finirà quando è stato soddisfatto il test per la precisione.
            adaptiveSubdivision(firstHalfCurve, nPoints);
            adaptiveSubdivision(secondHalfCurve, nPoints);
        }
    }
    //caso in cui possono tirare disegnare una linea dritta tra i due punti estremi in quanto il test di planarità è stato soddisfatto
    else
    {
        //imposto le coordinate dei due punti che saranno gli estremi del segmento da disegnare perché il test di planarità è stato soddisfatto

        pointsCurve[0][0] = firstPoint[0];
        pointsCurve[0][1] = firstPoint[1];
        pointsCurve[0][2] = 0;

        pointsCurve[1][0] = lastPoint[0];
        pointsCurve[1][1] = lastPoint[1];
        pointsCurve[1][2] = 0;

        glBindVertexArray(VAO_CURVE_POINTS);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_CURVE_POINTS);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pointsCurve), &pointsCurve[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glLineWidth(4);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
        glBindVertexArray(0);
        return;
    }
    return;
}

/*
Funzione che data in input alla glutKeyboardFunc

permmette di cancellare primo punto di controllo con f e ultimo punto
di controllo con l
*/

void myKeyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'f':
        if (nCurrentPoints > 0) {
            removeFirstPoint();
            glutPostRedisplay();
        }
        break;
    case 'l':
        if (nCurrentPoints > 0) {
            removeLastPoint();
            glutPostRedisplay();
        }
        break;
    case 27: // Escape key
        exit(0);
        break;
    }
}

/*
funzione usata in myKeyboardFunc (se viene premuto tasto f)
e in funzione addNewPoint se numero massimo di punti viene
ecceduto
*/
void removeFirstPoint()
{
    int i;
    if (nCurrentPoints > 0)
    {
        // Remove the first point, slide the rest down
        nCurrentPoints--;
        for (i = 0; i < nCurrentPoints; i++)
        {
            pointsInWindow[i][0] = pointsInWindow[i + 1][0];
            pointsInWindow[i][1] = pointsInWindow[i + 1][1];
            pointsInWindow[i][2] = 0;
        }
    }
}

/*
funzione usata se viene premuto il tasto l (elle) oppure
se viene sorprassato il numero massimo di punti ammissibili
*/
void removeLastPoint()
{
    if (nCurrentPoints > 0)
    {
        nCurrentPoints--;
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
tramite mouse.

La funzione mymouseFunc è una callback di glute mouse function e gli argomenti vengono passati direttamente da opengl,
quindi la funzione mymouse function viene solo passata alla glut mouse function e non chiamata in altre parti del codice.
*/
void checkPointClickorNew (int button, int state, int x, int y)
{

    /*condizione per controllare che tasto sinistro sia stato premuto e
    mantenuto giù
    con GLUT_LEFT_BUTTON controllo che sia stato premuto e con glut down
    che sia mantenuto giù
    */
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {

        //in questo caso la porzione di finestra è stata premuta per spostare un punto e non per crearne un altro
        if (mouseOverIndex != -1)
        {
            isMovingPoint = true;
            movingPoint = mouseOverIndex;
            return;
        }
        // (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
        float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
        float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

        //printf("new point Xpos %f Ypos %f \n", xPos, yPos);
        //printf("new pixel x %d y %d \n", x, y);
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

Questa funzione è una callback della funzione opengl glutPassiveMotionFunc, quindi viene direttamente
passata ad essa e non richiamata in altre parti del codice. I parametri xy vengo passati direttamente da opengl
*/
void myPassiveMotionFunction(int x, int y)
{
    //xy sono le coordinate del puntatore mouse al momento, da viewport devono essere trasformate a window
    float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
    float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

    //se uno dei punti nella finestra si avvicina di un tot alle coordinate del puntatore, allora si attiva il mouseOverIndex
    //per quel punto, infatti mouse over index ci dirà quale i-esimo punto è puntato al momento. se mouseoverindex è -1
    //allora vuol dire che nessun punto è vicino alle coordinate del puntatore del mouse
    for (int i = 0; i < nCurrentPoints; i++)
    {
        float dist = sqrt(pow(pointsInWindow[i][0] - xPos, 2) + pow(pointsInWindow[i][1] - yPos, 2));
        if (dist < 0.03)
        {
            mouseOverIndex = i;
            glutPostRedisplay();
            return;
        }
        else
        {
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
void spostaPunto(int x, int y)
{
    float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
    float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

    //se il punto è stato toccato per essere trascinato bisogna aggiornare le sue posizioni
    if (isMovingPoint)
    {
        pointsInWindow[movingPoint][0] = xPos;
        pointsInWindow[movingPoint][1] = yPos;
    }
    glutPostRedisplay();
}

/*
Funzione che permette di aggiungere un nuovo punto
alla fine della lista. Si rimuove il primo punto
nella lista se ci sono troppi punti.
*/
void addNewPoint(float x, float y)
{
    if (nCurrentPoints >= nMaxPoints)
    {
        removeFirstPoint();
    }
    pointsInWindow[nCurrentPoints][0] = x;
    pointsInWindow[nCurrentPoints][1] = y;
    pointsInWindow[nCurrentPoints][2] = 0;
    nCurrentPoints++;
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
nella init vanno tutte le cose che si fanno una volta sola, prima di renderizzare
*/
void init(void)
{

    //control polygon data
    glGenVertexArrays(1, &VAO_CONTROL_POLYGON);
    glBindVertexArray(VAO_CONTROL_POLYGON);
    glGenBuffers(1, &VBO_CONTROL_POLYGON);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_CONTROL_POLYGON);

    //curve points data
    glGenVertexArrays(1, &VAO_CURVE_POINTS);
    glBindVertexArray(VAO_CURVE_POINTS);
    glGenBuffers(1, &VBO_CURVE_POINTS);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_CURVE_POINTS);

    // Background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glViewport(0, 0, width, height);
}

//Algoritmo deCasteljau, parametri t e 
void deCasteljau(float t, float* curve)
{
    float curveX[nMaxPoints], curveY[nMaxPoints];

    for (int i = 0; i < nCurrentPoints; i++)
    {
        curveX[i] = pointsInWindow[i][0];
        curveY[i] = pointsInWindow[i][1];
    }
    for (int i = 1; i < nCurrentPoints; i++)
    {
        for (int k = 0; k < nCurrentPoints - i; k++)
        {
            curveX[k] = (1 - t) * curveX[k] + (t)*curveX[k + 1];
            curveY[k] = (1 - t) * curveY[k] + (t)*curveY[k + 1];
        }
    }
    curve[0] = curveX[0];
    curve[1] = curveY[0];
    curve[2] = 0.0;
}

/*
Questa funzione va in input alla funzione glut glutDisplayFunc

è la funzione che effettivamente fa la renderizzazione.

Come fa la drawArrays a sapere dove deve disegnare i punti?


*/

void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO_CONTROL_POLYGON);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_CONTROL_POLYGON);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointsInWindow), &pointsInWindow[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    //si disegnano prima i segmenti che uniscono i punti (line strip), poi i punti
    // Draw the line segments
    glLineWidth(1);
    glDrawArrays(GL_LINE_STRIP, 0, nCurrentPoints);

    // Draw the points (si disegnano i punti di controllo delle linee)
    glPointSize(16.0);
    glDrawArrays(GL_POINTS, 0, nCurrentPoints);
    glBindVertexArray(0);

    //se ci sono almeno due punti nella finestra allora si potranno appllicare o de casteljau o suddivisione adattiva in base alla scelta effettuata all'inizio
    if (nCurrentPoints > 1)
    {
        float curve[3];
        
        if (chosenModality == 1)
        {
            for (i = 0; i <= 100; i++)
            {
                //tramite i/100 parametrizzo la t tra 0 ed 1
                deCasteljau((float)i / 100, curve);
                //trattengo i punti xy da disegnare in seguito
                pointsCurve[i][0] = curve[0];
                pointsCurve[i][1] = curve[1];
                pointsCurve[i][2] = 0;
            }

            glBindVertexArray(VAO_CURVE_POINTS);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_CURVE_POINTS);
            glBufferData(GL_ARRAY_BUFFER, sizeof(pointsCurve), &pointsCurve[0], GL_STATIC_DRAW);
            //ci sono solo gli attributi di posizione
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            //si disegnano i punti interpolati con algoritmo de casteljau
            glLineWidth(4);
            glDrawArrays(GL_LINE_STRIP, 0, 101);
            glBindVertexArray(0);
        }
        //caso della suddivisione adattiva
        else if (chosenModality == 2)
        {
            {//per ogni punto devo prendere la x e y che metto in un array temporaneo che è la copia del mio pointsInWindow
                for (int i = 0; i < 2; i++)
                    for (int j = 0; j < nCurrentPoints; j++)
                        points[j][i] = pointsInWindow[j][i];

                //una volta caricato il vettore tempArray applico la suddivisione adattiva, al suo interno avverà anche la draw
                adaptiveSubdivision(points, nCurrentPoints);
            }
        }
    }
    glutSwapBuffers();
}

int main(int argc, char** argv)
{

    printf("digitare 1 per de casteljau, 2 for suddivisione adattiva: ");

    cin >> chosenModality;

    if (chosenModality == 2) {
        cout << "\nScegli una quota di planarita: ";
        cin >> precisionPlanarity;
    }



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
    glutMouseFunc(checkPointClickorNew);
    glutMotionFunc(spostaPunto);
    glutPassiveMotionFunc(myPassiveMotionFunction);

    glewExperimental = GL_TRUE;
    glewInit();

    initShader();
    init();

    glutMainLoop();
}

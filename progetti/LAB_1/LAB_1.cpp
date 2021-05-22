/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
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

bool pointerOverPoint = false;

float boundForPointer = 0.03;

int chosenModality;

static unsigned int programId;

unsigned int VAO_CONTROL_POLYGON;
unsigned int VBO_CONTROL_POLYGON;

unsigned int VAO_CURVE_POINTS;
unsigned int VBO_CURVE_POINTS;

using namespace glm;


int pointOfInterest;
bool isMovingPoint = false;
//variabile globale che rappresenta indice del punto che sto spostando
int movingPoint;

float pointsInWindow[nMaxPoints][3];

float pointsCurve[nMaxPoints][3];

// Window size in pixels
int width = 500;
int height = 500;

int nCurrentPoints = 0;
float points[nMaxPoints][3];

float subdivisionFactor = 0.5;

//parametro di precisione suddivisione adattiva
float precisionPlanarity;

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

                //applicazione decasteljau per ricavare i punti delle due sottocurve
                for (int j = 0; j < 2; j++)
                    for (int k = 0; k < nPoints-1; k++)
                    {
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
funzione usata in myKeyboardFunc (se viene premuto tasto f)
e in funzione addNewPoint se numero massimo di punti viene
ecceduto
*/
void deletePointFirst()
{
    if (nCurrentPoints > 0)
    {
        nCurrentPoints--;
        for (int i = 0; i < nCurrentPoints; i++)
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
void deletePointLast()
{
    if (nCurrentPoints > 0)
    {
        nCurrentPoints--;
    }
}

/*
Funzione che permette di aggiungere un nuovo punto
alla fine della lista. Si rimuove il primo punto
nella lista se ci sono troppi punti.
*/
void createPoint(float x, float y)
{
    if (nCurrentPoints >= nMaxPoints)
    {
        deletePointFirst();
    }
    pointsInWindow[nCurrentPoints][0] = x;
    pointsInWindow[nCurrentPoints][1] = y;
    pointsInWindow[nCurrentPoints][2] = 0;
    nCurrentPoints++;
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
            deletePointFirst();
            glutPostRedisplay();
        }
        break;
    case 'l':
        if (nCurrentPoints > 0) {
            deletePointLast();
            glutPostRedisplay();
        }
        break;
    case 27: // Escape key
        exit(0);
        break;
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

è una callback di glute mouse function e gli argomenti vengono passati direttamente da opengl,
quindi la funzione viene solo passata alla glut mouse function e non chiamata in altre parti del codice.
*/

void movePointOrCreateNew(int button, int state, int x, int y)
{
    /*condizione per controllare che tasto sinistro sia stato premuto e
    mantenuto giù
    con GLUT_LEFT_BUTTON controllo che sia stato premuto e con glut down
    che sia mantenuto giù
    */
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        //in questo caso la porzione di finestra è stata premuta per spostare un punto e non per crearne un altro
        if (pointerOverPoint)
        {
            isMovingPoint = true;
            movingPoint = pointOfInterest;
            return;
        }
        // (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
        float pointX = -1.0f + ((float)x) * 2 / ((float)(width));
        float pointY = -1.0f + ((float)(height - y)) * 2 / ((float)(height));
        createPoint(pointX, pointY);
        glutPostRedisplay();
    }
}



float distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/*
Funzione da dare in input alla glutPassiveMotionFunc

Controlla gli spostamenti passivi del mouse, cioè quelli che avvengono
senza che venga toccato un altro comando. Il compito di questa funzione
è quello di attivare il flag se si rileva che la distanza
del puntatore ed uno dei punti di controllo disegnati è sotto una certa soglia,
cioè il puntatore è approssimabilmente sopra il punto.

Questa funzione è una callback della funzione opengl glutPassiveMotionFunc, quindi viene direttamente
passata ad essa e non richiamata in altre parti del codice. I parametri xy vengo passati direttamente da opengl
*/

void checkClickedPoint(int x, int y)
{
    //xy sono le coordinate del puntatore mouse al momento, da viewport devono essere trasformate a window
    float xPoint = -1.0f + ((float)x) * 2 / ((float)(width));
    float yPoint = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

    //se uno dei punti nella finestra si avvicina di un tot alle coordinate del puntatore, allora si attiva il pointOfInterest
    //per quel punto, infatti mouse over index ci dirà quale i-esimo punto è puntato al momento. se pointOfInterest è -1
    //allora vuol dire che nessun punto è vicino alle coordinate del puntatore del mouse
    for (int i = 0; i < nCurrentPoints; i++)
    {
        if (distance(xPoint, yPoint, pointsInWindow[i][0], pointsInWindow[i][1]) < boundForPointer){
            pointOfInterest = i;
            pointerOverPoint = true;
            glutPostRedisplay();
            return;
        }
        else{
            pointerOverPoint = false;
        }
    }
    glutPostRedisplay();
}

/*
Funzione da dare in input alla glutMotionFunc

permette di aggiornare le posizioni de punti di controllo
mentre vengono spostati
*/
void movePoints(int x, int y)
{
    float pointX = -1.0f + ((float)x) * 2 / ((float)(width));
    float pointY = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

    //se il punto è stato toccato per essere trascinato bisogna aggiornare le sue posizioni
    if (isMovingPoint)
    {
        pointsInWindow[movingPoint][0] = pointX;
        pointsInWindow[movingPoint][1] = pointY;
    }
    glutPostRedisplay();
}


void initShader(void)
{
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShader.glsl";
    char* fragmentShader = (char*)"fragmentShader.glsl";

    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(programId);
}

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

//Algoritmo deCasteljau
void deCasteljau(float* resultDeCasteljau, float t)
{
    float pointX[nMaxPoints], pointY[nMaxPoints];

    for (int i = 0; i < nCurrentPoints; i++)
    {
        pointX[i] = pointsInWindow[i][0];
        pointY[i] = pointsInWindow[i][1];
    }
    for (int i = 1; i < nCurrentPoints; i++)
    {
        //le iterazioni lerp scenderanno di 1 ogni volta che finisce un ciclo estero. Esempio con poligonale
        //da quattro punti: prima 3 lerp, poi 2, poi una, fine
        for (int k = 0; k < nCurrentPoints - i; k++)
        {
            pointX[k] = (1 - t) * pointX[k] + (t)*pointX[k + 1];
            pointY[k] = (1 - t) * pointY[k] + (t)*pointY[k + 1];
        }
    }
    resultDeCasteljau[0] = pointX[0];
    resultDeCasteljau[1] = pointY[0];
    resultDeCasteljau[2] = 0.0;
}

void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO_CONTROL_POLYGON);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_CONTROL_POLYGON);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointsInWindow), &pointsInWindow[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //si disegnano prima i segmenti che uniscono i punti (la poligonale di controllo), poi i punti di controllo
    glLineWidth(1);
    glDrawArrays(GL_LINE_STRIP, 0, nCurrentPoints);
    glPointSize(16.0);
    glDrawArrays(GL_POINTS, 0, nCurrentPoints);
    glBindVertexArray(0);

    //se ci sono almeno due punti nella finestra allora si potranno appllicare o de casteljau o suddivisione adattiva in base alla scelta effettuata all'inizio
    if (nCurrentPoints > 1)
    {
        //caso algoritmo di De Casteljau
        if (chosenModality == 1)
        {
            float resultDeCasteljau[3];

            for (int i = 0; i <= 100; i++)
            {
                //tramite i/100 parametrizzo la t tra 0 ed 1
                deCasteljau(resultDeCasteljau, (float)i / 100);
                //trattengo i punti x ed y da disegnare in seguito
                pointsCurve[i][0] = resultDeCasteljau[0];
                pointsCurve[i][1] = resultDeCasteljau[1];
                pointsCurve[i][2] = 0;
            }

            glBindVertexArray(VAO_CURVE_POINTS);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_CURVE_POINTS);
            glBufferData(GL_ARRAY_BUFFER, sizeof(pointsCurve), &pointsCurve[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            //si disegnano i tratti di curva derivati dall'algoritmo di De Casteljau
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

                //una volta caricato il vettore applico la suddivisione adattiva, al suo interno avverà anche la draw
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

    glutPassiveMotionFunc(checkClickedPoint);
    glutMouseFunc(movePointOrCreateNew);
    glutMotionFunc(movePoints);


    glewExperimental = GL_TRUE;
    glewInit();

    initShader();
    init();

    glutMainLoop();
}

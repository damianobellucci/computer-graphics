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
#define MaxNumPts 120

int choosedDrawModality;

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

// Window size in pixels
int width = 500;
int height = 500;

int NumPts = 0;
int NumPtsS = 0;

float tempArray[MaxNumPts][3];
float costante_subdivision = 0.5;

//parametro di precisione suddivisione adattiva
float quota_planarita = 0.01;

int i, j, xy;
float x = 0, y = 0;
int numero_tratti;

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

void adaptiveSubdivision(float points[MaxNumPts][3], int nPoints)
{
    //test di planarità sui control point esterni. devo prendere i due punti più esterni per tirare la linea sulla quale dovrò calcolare la distanza per il test di planarità

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
        if (dist > quota_planarita)
            continueRecursion = true;
    }
    //caso in cui possono tirare disegnare una linea dritta tra i due punti estremi in quanto il test di planarità è stato soddisfatto
    if (continueRecursion) {
        {
            float firstHalfCurve[MaxNumPts][3];
            float secondHalfCurve[MaxNumPts][3];
            //carico i nuovi punti
            for (int i = 0; i < nPoints; i++)
            {
                firstHalfCurve[i][0] = points[0][0];
                firstHalfCurve[i][1] = points[0][1];
                secondHalfCurve[nPoints - i - 1][0] = points[nPoints - i - 1][0];
                secondHalfCurve[nPoints - i - 1][1] = points[nPoints - i - 1][1];

                //applicazione decasteljau per le due sottocurve per calcolare i punti intermedi di ogni sottocurva
                for (int j = 0; j < 2; j++)
                    for (int k = 0; k < nPoints; k++)
                    {
                        //calcolo della lerp
                        points[k][j] = points[k][j] * (1 - costante_subdivision) + points[k + 1][j] * costante_subdivision;
                    }
            }
            //vado in ricorsione sulle due sottocurve ottenute dividendo quella iniziale
            adaptiveSubdivision(firstHalfCurve, nPoints);
            adaptiveSubdivision(secondHalfCurve, nPoints);
        }
    }
    else
    {
        //imposto le coordinate dei due punti che saranno gli estremi del segmento da disegnare

        CurveArray[0][0] = firstPoint[0];
        CurveArray[0][1] = firstPoint[1];
        CurveArray[0][2] = 0;

        CurveArray[1][0] = lastPoint[0];
        CurveArray[1][1] = lastPoint[1];
        CurveArray[1][2] = 0;

        glBindVertexArray(VAO_2);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glLineWidth(2);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
        glBindVertexArray(0);
        numero_tratti++;
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
        if (NumPts > 0) {
            removeFirstPoint();
            glutPostRedisplay();
        }
        break;
    case 'l':
        if (NumPts > 0) {
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
    if (NumPts > 0)
    {
        // Remove the first point, slide the rest down
        NumPts--;
        for (i = 0; i < NumPts; i++)
        {
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
void removeLastPoint()
{
    if (NumPts > 0)
    {
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
    for (int i = 0; i < NumPts; i++)
    {
        float dist = sqrt(pow(PointArray[i][0] - xPos, 2) + pow(PointArray[i][1] - yPos, 2));
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
void addNewPoint(float x, float y)
{
    if (NumPts >= MaxNumPts)
    {
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
nella init vanno tutte le cose che si fanno una volta sola, prima di renderizzare
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

void deCasteljau(float t, float* result)
{
    int i, k;
    float coordX[MaxNumPts], coordY[MaxNumPts];

    for (i = 0; i < NumPts; i++)
    {
        coordX[i] = PointArray[i][0];
        coordY[i] = PointArray[i][1];
    }

    for (i = 1; i < NumPts; i++)
    {
        for (k = 0; k < NumPts - i; k++)
        {
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


    //si disegnano prima i segmenti che uniscono i punti (line strip), poi i punti
    // Draw the line segments
    glLineWidth(2.5);
    glDrawArrays(GL_LINE_STRIP, 0, NumPts);

    // Draw the points (si disegnano i punti di controllo delle linee)
    glPointSize(8.0);
    glDrawArrays(GL_POINTS, 0, NumPts);
    glBindVertexArray(0);

    //se ci sono almeno due punti nella finestra allora si potranno appllicare o de casteljau o suddivisione adattiva in base alla scelta effettuata all'inizio
    if (NumPts > 1)
    {
        float result[3];
        
        if (choosedDrawModality == 0)
        {
            for (i = 0; i <= 100; i++)
            {
                deCasteljau((GLfloat)i / 100, result);
                CurveArray[i][0] = result[0];
                CurveArray[i][1] = result[1];
                CurveArray[i][2] = 0;
            }

            glBindVertexArray(VAO_2);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
            //ci sono solo gli attributi di posizione
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            //si disegnano i punti interpolati con algoritmo de casteljau
            glLineWidth(0.5);
            glDrawArrays(GL_LINE_STRIP, 0, 101);
            glBindVertexArray(0);
        }
        //caso della suddivisione adattiva
        else if (choosedDrawModality == 1)
        {

            {//per ogni punto devo prendere la x e y che metto in un array temporaneo che è la copia del mio pointarray
                for (xy = 0; xy < 2; xy++)
                    for (j = 0; j < NumPts; j++)
                        tempArray[j][xy] = PointArray[j][xy];
                numero_tratti = 0;
                //una volta caricato il vettore tempArray applico la suddivisione adattiva, al suo interno avverà anche la draw
                adaptiveSubdivision(tempArray, NumPts);
            }
        }
    }
    glutSwapBuffers();
}

int main(int argc, char** argv)
{

    printf("digitare 0 per de casteljau, 1 for suddivisione adattiva: ");

    cin >> choosedDrawModality;

    if (choosedDrawModality == 1) {
        cout << "\nScegli una quota di planarita: ";
        cin >> quota_planarita;
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

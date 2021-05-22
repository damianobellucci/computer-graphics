#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <chrono>
#include <thread>

vector<int> posXPrecedentePuntiPS = {};
vector<int> posYPrecedentePuntiPS = {};
vector<int> fattoreVelocitaParticellaPS = {};

bool attivaPS = false;
int vitaPS = 0;
int posxPS;
int posyPS;

string direzioneScia;
int nPuntiScia = 30;
int vitaScia = 0;

float varOffsetSciaX;
float varOffsetSciaY;

float dimensionePalla = 16;
int triangoliPalla = 9;
float raggioxPalla = 1;
float raggioyPalla = 1;
float dimensioneNemici = 60;

int larghezzaGiocatore = 200;
int altezzaGiocatore = 25;

int checkSwapBuffer = 0;

int window;
int counterColpiti = 0;
bool proiettileColpitoAvversario = false;

vector<int> posizioniXinizialiNemici = {};
vector<int> posizioniYinizialiNemici = {};
vector<bool> avversarioColpito = {};
vector<bool> avversarioDisattivato = {};
vector<float> fattoreDispersionePuntiScia = {};

void offsetSciaX() {
	if (direzioneScia == "left") {
		varOffsetSciaX = dimensionePalla / 2;
	}
	else varOffsetSciaX = -dimensionePalla / 2;
}

void offsetSciaY() {
	varOffsetSciaY = -dimensionePalla / 2;
}


int rangeRandomAlg2(int min, int max) {
	int n = max - min + 1;
	int remainder = RAND_MAX % n;
	int x;
	do {
		x = rand();
	} while (x >= RAND_MAX - remainder);
	return min + x % n;
}

int rangeRandomAlgNuovo(int min, int max) {
	int n = max - min + 1;
	int remainder = RAND_MAX % n;
	int x;
	do {
		x = rand();
	} while (x >= RAND_MAX - remainder);
	if ((min + x % n) <= min / 2)
		return -min + x % n;
	return min + x % n;
}


int numeroNemici = 5;

static unsigned int programId, programId_1;
#define PI 3.14159265358979323846

unsigned int VAO, VAO_CIELO, VAO_NEMICO, VAO_PALLA, VAO_SCIA;
unsigned int VBO, VBO_C, VBO_N, loc, MatProj, MatModel, MatProj1, MatModel1, VBO_PALLA, VBO_SCIA;

// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

vec4 col_bianco = { 1.0,1.0,1.0, 1.0 };
vec4 col_rosso = { 1.0,0.0,0.0, 1.0 };
vec4 col_nero = { 0.0,0.0,0.0, 1.0 };
vec4 col_magenta = { 1.0,1.0,0.0, 1.0 };

int NumeroColpiti = 0;
mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento di sistema di riferimento: da Sistema diriferimento dell'oggetto a sistema di riferimento nel Mondo
typedef struct { float x, y, z, r, g, b, a; } Point;

float dxnemici = 0;
float dynemici = 0;
float posxN, posyN;
int nemici_per_riga = 1;
int numero_di_righe = 2;
int nTriangles = 15;

int nVertices_Navicella = 12 * nTriangles + 1;


int nvBocca = 5;
int nvTentacoli = 16;
int nVertices_Nemico = 3 * nTriangles + 2 * 3 * nTriangles + nvBocca + nvTentacoli;
int nVertices_PallaProiettile = 24;

Point* Navicella = new Point[7];
Point* Nemico = new Point[6];
Point* giocatoreRettangolare = new Point[7];
Point* PallaProiettile = new Point[nVertices_PallaProiettile];

Point* puntiScia = new Point[nPuntiScia];
int vertices_cielo = 6;
Point* Cielo = new Point[vertices_cielo];

// Viewport size
int width = 1280;
int height = 720;

//variabile di tempo, utile magari per fare cose dinamiche
float t;

//posizione relativa del proiettile rispetto alla navicella
float posx_Proiettile = larghezzaGiocatore / 2, posy_Proiettile = altezzaGiocatore;

//ANIMARE V
double VelocitaOrizzontale = 0; //velocita orizzontale (pixel per frame)
int scuoti = 0;
double accelerazione = 1; // forza di accelerazione data dalla tastiera
float posx = width / 2 - larghezzaGiocatore / 2; //coordinate sul piano della posizione iniziale della navicella
float posy = height * 0.225;
float posizione_di_equilibrio = posy;
float angolo = 0;

bool pressing_up = true;
bool pressing_down = true;

bool pressing_left = false;
bool pressing_right = false;
bool pressing_attack = false;
bool pressing_rotate_s = false;
bool pressing_rotate_d = false;

bool** colpito;
float angoloV = 0;
double range_fluttuazione = 0; // fluttuazione su-gi 
double angle = 0; // angolo di fluttuazione
double angle_offset = 10; // quanto   accentuata l'oscillazione angolare
double float_yoffset = 0; // distacco dalla posizione d'equilibrio 
int frame_animazione = 0; // usato per animare la fluttuazione
int frame = 0;
bool start = 0;


int firePosition;
bool sparoInVolo = false;

float lerp(float a, float b, float t) {
	//Interpolazione lineare tra a e b secondo amount
	return (1 - t) * a + t * b;
}

double  degtorad(double angle) {
	return angle * PI / 180;
}

vector<int> sfasamentoXnemici;
vector<int> sfasamentoYnemici;

vector<int> direzioneXnemici;
vector<int> direzioneYnemici;

void initScia(Point* puntiScia) {

	for (int i = 0; i < nPuntiScia; i++) {

		puntiScia[i].x = 0;
		puntiScia[i].y = 0;
		puntiScia[i].z = 0;
		puntiScia[i].r = 0;
		puntiScia[i].g = 0;
		puntiScia[i].b = 0;
		puntiScia[i].a = 0;
	}
}

void disegna_palla(float cx, float cy, float raggiox, float raggioy, vec4 color_top, vec4 color_bot, Point* Cerchio)
{
	int i;
	float stepA = (2 * PI) / triangoliPalla;

	int comp = 0;
	for (i = 0; i < triangoliPalla; i++)
	{
		Cerchio[comp].x = cx + cos((double)i * stepA) * raggiox;
		Cerchio[comp].y = cy + sin((double)i * stepA) * raggioy;
		Cerchio[comp].z = 0.0;
		Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;

		Cerchio[comp + 1].x = cx + cos((double)(i + 1) * stepA) * raggiox;
		Cerchio[comp + 1].y = cy + sin((double)(i + 1) * stepA) * raggioy;
		Cerchio[comp + 1].z = 0.0;
		Cerchio[comp + 1].r = color_top.r; Cerchio[comp + 1].g = color_top.g; Cerchio[comp + 1].b = color_top.b; Cerchio[comp + 1].a = color_top.a;

		Cerchio[comp + 2].x = cx;
		Cerchio[comp + 2].y = cy;
		Cerchio[comp + 2].z = 0.0;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;

		comp += 3;
	}
}

void updateNemici(int value)
{
	int bound = 60;
	int distaccoDaParete = 20;
	int entitaRimbalzo = 25;

	frame++;
	if (frame % 30 == 0)
	{

		for (int i = 0; i < numeroNemici; i++) {
			sfasamentoXnemici.at(i) = (rand() % 10);
			sfasamentoYnemici.at(i) = (rand() % 10);
		}

		for (int i = 0; i < numeroNemici; i++) {
			if ((posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i)) + bound > width - distaccoDaParete || (posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i) <= distaccoDaParete)) {
				if ((posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i)) + bound > width - distaccoDaParete)
				{
					direzioneXnemici.at(i) = direzioneXnemici.at(i) - entitaRimbalzo;
				}
				else {
					direzioneXnemici.at(i) = direzioneXnemici.at(i) + entitaRimbalzo;
				}
			}
			else {
				int offset = 0;
				if (rand() % 10 < 5) {
					offset = -10;
				}
				else offset = +10;
				direzioneXnemici.at(i) = direzioneXnemici.at(i) + offset;
			}
			if ((posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i)) + bound > height - distaccoDaParete || (posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i)) < distaccoDaParete) {
				if ((posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i)) + bound > height - distaccoDaParete) {
					direzioneYnemici.at(i) = direzioneYnemici.at(i) - entitaRimbalzo;
				}
				else {
					direzioneYnemici.at(i) = direzioneYnemici.at(i) + entitaRimbalzo;
				}
			}

			else {
				int offset = 0;
				if (rand() % 10 < 5) {
					offset = -10;
				}
				else offset = +10;
				direzioneYnemici.at(i) = direzioneYnemici.at(i) + offset;
			}
		}
	}
	glutTimerFunc(5, updateNemici, 0);
	glutPostRedisplay();
}

double  radtodeg(double angle) {
	return angle / (PI / 180);
}

void update(int a)
{

	float timeValue = glutGet(GLUT_ELAPSED_TIME);
	t = (sin(timeValue));

	//printf("Valore di t %f \n", t);
	glutPostRedisplay();
	glutTimerFunc(50, update, 0);
}


double radParabolic = PI / 4;
double gravityParabolic = 5;
double v0Parabolic = 100;
double counterParabolic = 0;
double counterParabolicRight = 0;
double x0Parabolic = larghezzaGiocatore / 2;
double y0Parabolic = altezzaGiocatore;

int parabolicX(int v0, int t, double rad)
{
	return (v0 * t * sin(rad));
}

int parabolicY(int v0, int t, double rad, double g)
{
	return (v0 * t * sin(rad)) - (0.5 * g * t * t);
}

void updateProiettile2r(int value)
{
	counterParabolicRight++;
	//Ascissa del proiettile durante lo sparo
	posx_Proiettile = x0Parabolic - parabolicX(v0Parabolic, counterParabolicRight / 15, radParabolic);
	//Ordinata del proettile durante lo sparo
	posy_Proiettile = y0Parabolic + parabolicY(v0Parabolic, counterParabolicRight / 15, radParabolic, gravityParabolic);

	/*
	cout << firePosition;
	cout << " | ";
	cout << posx_Proiettile;
	cout << " | ";
	cout << posy_Proiettile;
	cout << "\n";
	*/

	//L'animazione deve avvenire finchè  l'ordinata del proiettile raggiunge un certo valore fissato e se non colpisce nessun avversario
	if (abs(posy_Proiettile) <= 500 && (firePosition + (posx_Proiettile)) >= 0 && !proiettileColpitoAvversario)
		glutTimerFunc(5, updateProiettile2r, 0);
	else {
		/*
		cout << posx_Proiettile;
		cout << " | ";
		cout << posy_Proiettile;
		cout << "\n";
		*/
		posy_Proiettile = altezzaGiocatore;
		posx_Proiettile = larghezzaGiocatore / 2;
		counterParabolicRight = 0;
		firePosition = posx;
		sparoInVolo = false;
		proiettileColpitoAvversario = false;
		vitaScia = 0;
		initScia(puntiScia);
	}


	glutPostRedisplay();
}

void updateProiettile2(int value)
{
	counterParabolic++;
	//Ascissa del proiettile durante lo sparo
	posx_Proiettile = x0Parabolic + parabolicX(v0Parabolic, counterParabolic / 15, radParabolic);
	//Ordinata del proettile durante lo sparo
	posy_Proiettile = y0Parabolic + parabolicY(v0Parabolic, counterParabolic / 15, radParabolic, gravityParabolic);

	/*
	cout << firePosition;
	cout << " | ";
	cout << posx_Proiettile;
	cout << " | ";
	cout << posy_Proiettile;
	cout << "\n";
	*/


	//L'animazione deve avvenire finchè  l'ordinata del proiettile raggiunge un certo valore fissato
	if (abs(posy_Proiettile) <= 500 && abs(firePosition + posx_Proiettile) <= width && !proiettileColpitoAvversario)
		glutTimerFunc(5, updateProiettile2, 0);
	else {
		/*
		cout << posx_Proiettile;
		cout << " | ";
		cout << posy_Proiettile;
		cout << "\n";
		*/
		posy_Proiettile = altezzaGiocatore;
		posx_Proiettile = larghezzaGiocatore / 2;
		counterParabolic = 0;
		firePosition = posx;
		sparoInVolo = false;
		proiettileColpitoAvversario = false;
		vitaScia = 0;
		initScia(puntiScia);
	}


	glutPostRedisplay();
}


void updateProiettile(int value)
{

	//Ascissa del proiettile durante lo sparo
	posx_Proiettile = 0;
	//Ordinata del proettile durante lo sparo
	posy_Proiettile++;


	//L'animazione deve avvenire finchè  l'ordinata del proiettile raggiunge un certo valore fissato
	if (posy_Proiettile <= 500)
		glutTimerFunc(5, updateProiettile, 0);
	else
		posy_Proiettile = 0;

	glutPostRedisplay();
}

void updateV(int a)
{
	bool moving = false;

	if (pressing_left)
	{
		VelocitaOrizzontale -= accelerazione;
		moving = true;
	}

	if (pressing_right)
	{
		VelocitaOrizzontale += accelerazione;
		moving = true;
	}

	if (float_yoffset >= 0) {
		frame_animazione += 6;
		if (frame_animazione >= 360) {
			frame_animazione -= 360;
		}
	}

	if (!moving) {

		if (VelocitaOrizzontale > 0)
		{
			VelocitaOrizzontale -= 1;
			if (VelocitaOrizzontale < 0)
				VelocitaOrizzontale = 0;
		}

		if (VelocitaOrizzontale < 0)
		{
			VelocitaOrizzontale += 1;
			if (VelocitaOrizzontale > 0)
				VelocitaOrizzontale = 0;
		}
	}
	if (pressing_rotate_s)
	{
		angoloV++;
		moving = true;
	}
	if (pressing_rotate_d)
	{
		angoloV--;
		moving = true;
	}
	//Aggioramento della posizione in x della navicella, che regola il movimento orizzontale
	posx += VelocitaOrizzontale;
	// Gestione Bordi viewport:
	// Se la navicella assume una posizione in x minore di 0 o maggiore di width dello schermo
	// facciamo rimbalzare la navicella ai bordi dello schermo
	if (posx < 0) {
		posx = 0;
		VelocitaOrizzontale = -VelocitaOrizzontale * 0.8;
	}
	if (posx + larghezzaGiocatore > width) {
		posx = width - larghezzaGiocatore;
		VelocitaOrizzontale = -VelocitaOrizzontale * 0.8;
	}
	// calcolo y come somma dei seguenti contributi: pos. di equilibrio, oscillazione periodica
	posy = posizione_di_equilibrio + sin(degtorad(frame_animazione)) * range_fluttuazione;
	angolo = cos(degtorad(frame_animazione)) * angle_offset - VelocitaOrizzontale * 1.3;
	glutPostRedisplay();
	glutTimerFunc(15, updateV, 0);
}


void keyboardPressedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		if (!sparoInVolo) {
			pressing_attack = true;
			updateProiettile2r(0);
			firePosition = posx;
			sparoInVolo = true;
			direzioneScia = "left";
			offsetSciaX();
			offsetSciaY();
		}
		break;
	case 'e':
		if (!sparoInVolo) {
			pressing_attack = true;
			updateProiettile2(0);
			firePosition = posx;
			sparoInVolo = true;
			direzioneScia = "right";
			offsetSciaX();
			offsetSciaY();
		}
		break;
	case 'a':
		pressing_left = true;
		break;
	case 'd':
		pressing_right = true;
		break;
	case 'r':
		pressing_rotate_s = true;
		break;
	case 'f':
		pressing_rotate_d = true;
		break;
	case 'w':
		pressing_up = true;
		break;
	case 's':
		pressing_down = true;
		break;
	case 'p':
		if (!sparoInVolo && radParabolic < 1.3854) {
			radParabolic = radParabolic + 0.1;
			/*
			cout << "\n";
			cout << radParabolic;
			cout << "\n";
			*/
		}
		break;
	case 'l':
		if (!sparoInVolo && radParabolic > 0.185399) {
			radParabolic = radParabolic - 0.1;
		}
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		pressing_attack = false;
		break;
	case 'e':
		pressing_attack = false;
		break;
	case 'a':
		pressing_left = false;
		break;
	case 'd':
		pressing_right = false;
		break;
	case 'r':
		pressing_rotate_s = false;
		break;
	case 'f':
		pressing_rotate_d = false;
		break;
	case 'w':
		pressing_up = false;
		break;
	case 's':
		pressing_down = false;
		break;
	default:
		break;
	}
}



/// /////////////////////////////////// Disegna geometria //////////////////////////////////////


void disegna_piano(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Point* piano)
{
	piano[0].x = x;	piano[0].y = y; piano[0].z = 0;
	piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
	piano[1].x = x + width;	piano[1].y = y;	piano[1].z = 0;
	piano[1].r = color_top.r; piano[1].g = color_top.g; piano[1].b = color_top.b; piano[1].a = color_top.a;
	piano[2].x = x + width;	piano[2].y = y + height; piano[2].z = 0;
	piano[2].r = color_bot.r; piano[2].g = color_bot.g; piano[2].b = color_bot.b; piano[2].a = color_bot.a;

	piano[3].x = x + width;	piano[3].y = y + height; piano[3].z = 0;
	piano[3].r = color_bot.r; piano[3].g = color_bot.g; piano[3].b = color_bot.b; piano[3].a = color_bot.a;
	piano[4].x = x;	piano[4].y = y + height; piano[4].z = 0;
	piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
	piano[5].x = x;	piano[5].y = y; piano[5].z = 0;
	piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;
}

float dimensioni_rettangolox = 1;
float dimensioni_rettangoloy = 1;

void disegna_rettangolo_giocatore(float cx, float cy, float raggiox, float raggioy, vec4 color_top, vec4 color_bot, Point* GiocatoreRettangolare)
{

	int comp = 0;
	for (int i = 0; i < 1; i++)
	{
		//primo vertice triangolo
		GiocatoreRettangolare[comp].x = cx;
		GiocatoreRettangolare[comp].y = cy;
		GiocatoreRettangolare[comp].z = 0.0;
		GiocatoreRettangolare[comp].r = color_bot.r; GiocatoreRettangolare[comp].g = color_bot.g; GiocatoreRettangolare[comp].b = color_bot.b; GiocatoreRettangolare[comp].a = color_bot.a;

		//secondo  vertice triangolo
		GiocatoreRettangolare[comp + 1].x = cx + dimensioni_rettangolox;
		GiocatoreRettangolare[comp + 1].y = cy;
		GiocatoreRettangolare[comp + 1].z = 0.0;
		GiocatoreRettangolare[comp + 1].r = color_top.r; GiocatoreRettangolare[comp + 1].g = color_top.g; GiocatoreRettangolare[comp + 1].b = color_top.b; GiocatoreRettangolare[comp + 1].a = color_top.a;

		//terzo  vertice triangolo
		GiocatoreRettangolare[comp + 2].x = cx;
		GiocatoreRettangolare[comp + 2].y = cy + dimensioni_rettangoloy;
		GiocatoreRettangolare[comp + 2].z = 0.0;
		GiocatoreRettangolare[comp + 2].r = color_top.r; GiocatoreRettangolare[comp + 2].g = color_top.g; GiocatoreRettangolare[comp + 2].b = color_top.b; GiocatoreRettangolare[comp + 2].a = color_top.a;

		comp += 3;
	}


	for (int i = 0; i < 1; i++)
	{
		//primo vertice triangolo
		GiocatoreRettangolare[comp].x = cx + dimensioni_rettangolox;
		GiocatoreRettangolare[comp].y = cy + dimensioni_rettangoloy;
		GiocatoreRettangolare[comp].z = 0.0;
		GiocatoreRettangolare[comp].r = color_bot.r; GiocatoreRettangolare[comp].g = color_bot.g; GiocatoreRettangolare[comp].b = color_bot.b; GiocatoreRettangolare[comp].a = color_bot.a;

		//secondo  vertice triangolo
		GiocatoreRettangolare[comp + 1].x = cx + dimensioni_rettangolox;
		GiocatoreRettangolare[comp + 1].y = cy;
		GiocatoreRettangolare[comp + 1].z = 0.0;
		GiocatoreRettangolare[comp + 1].r = color_top.r; GiocatoreRettangolare[comp + 1].g = color_top.g; GiocatoreRettangolare[comp + 1].b = color_top.b; GiocatoreRettangolare[comp + 1].a = color_top.a;

		//terzo  vertice triangolo
		GiocatoreRettangolare[comp + 2].x = cx;
		GiocatoreRettangolare[comp + 2].y = cy + dimensioni_rettangoloy;
		GiocatoreRettangolare[comp + 2].z = 0.0;
		GiocatoreRettangolare[comp + 2].r = color_top.r; GiocatoreRettangolare[comp + 2].g = color_top.g; GiocatoreRettangolare[comp + 2].b = color_top.b; GiocatoreRettangolare[comp + 2].a = color_top.a;

		comp += 3;
	}

	GiocatoreRettangolare[comp].x = 0;
	GiocatoreRettangolare[comp].y = 0;
	GiocatoreRettangolare[comp].z = 0;
	GiocatoreRettangolare[comp].r = 1;
	GiocatoreRettangolare[comp].g = 1;
	GiocatoreRettangolare[comp].b = 1;
	GiocatoreRettangolare[comp].a = 1;
}

void disegnaNemico(float cx, float cy, float raggiox, float raggioy, vec4 color_top, vec4 color_bot, Point* Cerchio)
{

	float stepA = (2 * PI) / nTriangles;



	int comp = 0;
	for (int i = 0; i < 1; i++)
	{
		//primo vertice triangolo
		Cerchio[comp].x = cx;
		Cerchio[comp].y = cy;
		Cerchio[comp].z = 0.0;
		Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;

		//secondo  vertice triangolo
		Cerchio[comp + 1].x = cx;
		Cerchio[comp + 1].y = cy +1;
		Cerchio[comp + 1].z = 0.0;
		Cerchio[comp + 1].r = color_bot.r; Cerchio[comp + 1].g = color_bot.g; Cerchio[comp + 1].b = color_bot.b; Cerchio[comp + 1].a = color_bot.a;

		//terzo  vertice triangolo
		Cerchio[comp + 2].x = cx + 1;
		Cerchio[comp + 2].y = cy;
		Cerchio[comp + 2].z = 0.0;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;

		comp += 3;
	}

	for (int i = 0; i < 1; i++)
	{
		//primo vertice triangolo
		Cerchio[comp].x = cx + 1;
		Cerchio[comp].y = cy;
		Cerchio[comp].z = 0.0;
		Cerchio[comp].r = color_bot.r; Cerchio[comp].g = color_bot.g; Cerchio[comp].b = color_bot.b; Cerchio[comp].a = color_bot.a;

		//secondo  vertice triangolo
		Cerchio[comp + 1].x = cx;
		Cerchio[comp + 1].y = cy + 1;
		Cerchio[comp + 1].z = 0.0;
		Cerchio[comp + 1].r = color_bot.r; Cerchio[comp + 1].g = color_bot.g; Cerchio[comp + 1].b = color_bot.b; Cerchio[comp + 1].a = color_bot.a;

		//terzo  vertice triangolo
		Cerchio[comp + 2].x = cx + 1;
		Cerchio[comp + 2].y = cy + 1;
		Cerchio[comp + 2].z = 0.0;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_top.a;

		comp += 3;
	}
}

void disegna_nemico(vec4 color_top_Nemico, vec4 color_bot_Nemico, vec4 color_top_Occhio, vec4 color_bot_Occhio, Point* Nemico)
{
	disegnaNemico(0.0, 0.0, 1.0, 1.0, color_top_Nemico, color_bot_Nemico, Nemico);
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

	char* vertexShader1 = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader1 = (char*)"fragmentShader_C_M_1.glsl";

	programId_1 = ShaderMaker::createProgram(vertexShader1, fragmentShader1);
}

void init(void)
{


	for (int i = 0; i < nPuntiScia; i++) {
		fattoreVelocitaParticellaPS.push_back(0);
	}


	for (int i = 0; i < nPuntiScia; i++) {
		posXPrecedentePuntiPS.push_back(0);
		posYPrecedentePuntiPS.push_back(0);
	}

	for (int i = 0; i < numeroNemici; i++) {
		avversarioColpito.push_back(false);
	}

	srand(time(NULL));
	for (int i = 0; i < numeroNemici; i++) {
		posizioniXinizialiNemici.push_back(rangeRandomAlg2(50, width - 50));
		posizioniYinizialiNemici.push_back(rangeRandomAlg2(400, height - 100));

	}

	for (int i = 0; i < numeroNemici; i++) {
		sfasamentoXnemici.push_back(0);
		sfasamentoYnemici.push_back(0);
	}

	for (int i = 0; i < numeroNemici; i++) {

		direzioneXnemici.push_back(-10);
		direzioneYnemici.push_back(-10);
	}

	for (int i = 0; i < numeroNemici; i++) {
		avversarioDisattivato.push_back(false);
	}

	for (int i = 0; i < nPuntiScia; i++) {
		int rand = rangeRandomAlg2(7, 100) / 100;
		fattoreDispersionePuntiScia.push_back(rand);
	}

	//////INIZIALIZZAZIONE SCIA PS//////////////////////////////////////

	glGenVertexArrays(1, &VAO_SCIA);
	glBindVertexArray(VAO_SCIA);
	glGenBuffers(1, &VBO_SCIA);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_SCIA);
	glBufferData(GL_ARRAY_BUFFER, nPuntiScia * sizeof(Point), &puntiScia[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	////////////////////////////////////////////


	vector<int> timeInstant{ 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	//Disegno SPAZIO/CIELO
	vec4 col_top = { 0.0,0.0,1,1 };
	vec4 col_bot = { 1,1,1,1 };

	disegna_piano(0.0, 0.0, 1.0, 1.0, col_bot, col_top, Cielo);
	//Genero un VAO
	glGenVertexArrays(1, &VAO_CIELO);
	//Ne faccio il bind (lo collego, lo attivo)
	glBindVertexArray(VAO_CIELO);
	//AL suo interno genero un VBO
	glGenBuffers(1, &VBO_C);
	//Ne faccio il bind (lo collego, lo attivo, assegnandogli il tipo GL_ARRAY_BUFFER)
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	//Carico i dati vertices sulla GPU
	glBufferData(GL_ARRAY_BUFFER, vertices_cielo * sizeof(Point), &Cielo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);


	//////////////////////////////////////////////////////////////
	//disegno palla protiettile
	vec4 color_top_corpo = { 1.0,0.0,0,1 };
	vec4 color_bot_corpo = { 1.0,1.0,1.0,1.0 };


	disegna_palla(0.0, -1.0, raggioxPalla, raggioyPalla, color_top_corpo, color_bot_corpo, PallaProiettile);

	glGenVertexArrays(1, &VAO_PALLA);
	glBindVertexArray(VAO_PALLA);
	glGenBuffers(1, &VBO_PALLA);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PALLA);
	glBufferData(GL_ARRAY_BUFFER, triangoliPalla * 3 * sizeof(Point), &PallaProiettile[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);


	//////////////////////////////////////////////////////////////

	//Disegno NAVICELLA
	vec4 color_top_Navicella = { 0,0,0,1.0 };
	vec4 color_bot_Navicella = { 0.5,0.9,1.0,0.9 };
	vec4 color_top_Corpo = { 0.0,0.5,0.8,1.0 };
	vec4 color_bot_Corpo = { 0.0,0.2,0.5,1.0 };
	vec4 color_top_Oblo = { 0.2,0.9,0.1,1.0 };
	vec4 color_bot_Oblo = { 0.0,0.2,0.8,1.0 };

	disegna_rettangolo_giocatore(0.0, 0.0, 1.0, 1.0, color_top_Navicella, color_bot_Navicella, giocatoreRettangolare);

	//Genero un VAO navicella
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 7 * sizeof(Point), &giocatoreRettangolare[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);

	vec4 color_top_Nemico = { 1.0,1.0,1.0,0.3 };
	vec4 color_top_Occhio = { 1.0,1.0,1.0,0.3 };

	disegna_nemico(color_top_Nemico, col_bianco, color_top_Occhio, col_bianco, Nemico);
	//Genero un VAO

	glGenVertexArrays(1, &VAO_NEMICO);
	glBindVertexArray(VAO_NEMICO);
	glGenBuffers(1, &VBO_N);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_N);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Point), &Nemico[0], GL_STATIC_DRAW);
	//per le posizioni xyz del vertice
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//per i colori e trasparenza rgba
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);

	//inizializzazione array colpito
	colpito = new bool* [numero_di_righe];
	for (int i = 0; i < numero_di_righe; i++)
		colpito[i] = new bool[nemici_per_riga];
	for (int i = 0; i < numero_di_righe; i++)
		for (int j = 0; j < nemici_per_riga; j++)
		{
			colpito[i][j] = false;
			//printf("%s", colpito[i][j] ? "true \n" : "false \n");
		}
	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(1.0, 0.5, 0.0, 1.0);

	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
	MatProj1 = glGetUniformLocation(programId_1, "Projection");
	MatModel1 = glGetUniformLocation(programId_1, "Model");
	loc = glGetUniformLocation(programId_1, "t");
}

bool firstDrawVirus = false;

void updatePS2(int v) {
	float v0 = 0.5;
	float a = 0.5;
	float tempo = vitaPS ;

	if (vitaPS == 0) {
		for (int i = 0; i < nPuntiScia; i++) {
			if (direzioneScia == "left") {
				posXPrecedentePuntiPS.at(i) = 0 - rangeRandomAlgNuovo(0 , dimensioneNemici);
				posYPrecedentePuntiPS.at(i) = 0 + rangeRandomAlgNuovo(0, dimensioneNemici );
			}
			else {
				posXPrecedentePuntiPS.at(i) = 0 + rangeRandomAlgNuovo(0, dimensioneNemici );
				posYPrecedentePuntiPS.at(i) = 0 + rangeRandomAlgNuovo(0, dimensioneNemici );
			}
		}
	}

	for (int i = 0; i < nPuntiScia; i++) {
		if (direzioneScia == "left") {

			puntiScia[i].x = posXPrecedentePuntiPS.at(i) - v0 * tempo - 1 / 2 * a * tempo * tempo;
			puntiScia[i].y = posYPrecedentePuntiPS.at(i) + v0 * tempo + 1 / 2 * a * tempo * tempo;
			posXPrecedentePuntiPS.at(i) = puntiScia[i].x;
			posYPrecedentePuntiPS.at(i) = puntiScia[i].y;
			puntiScia[i].x = puntiScia[i].x + posxPS;
			puntiScia[i].y = puntiScia[i].y + posyPS;
		}
		else {
			puntiScia[i].x = posXPrecedentePuntiPS.at(i) + v0 * tempo + 1 / 2 * a * tempo * tempo;
			puntiScia[i].y = posYPrecedentePuntiPS.at(i) + v0 * tempo + 1 / 2 * a * tempo * tempo;
			posXPrecedentePuntiPS.at(i) = puntiScia[i].x;
			posYPrecedentePuntiPS.at(i) = puntiScia[i].y;
			puntiScia[i].x = puntiScia[i].x + posxPS;
			puntiScia[i].y = puntiScia[i].y + posyPS;
		}

		puntiScia[i].z = 0.0;
		puntiScia[i].r = 1.0;
		puntiScia[i].g = 0;
		puntiScia[i].b = 0;
		puntiScia[i].a = 1;
	}


	if (vitaPS < 20) {
		vitaPS++;
		glutPostRedisplay();
		glutTimerFunc(8, updatePS2, 0);
	}
	else {
		attivaPS = false;
		vitaPS = 0;
	}

}

void drawScene(void)
{
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(programId);  // attiva fragment shader basic per tutta la scena meno i nemici 

	// Disegna cielo
	glBindVertexArray(VAO_CIELO);
	Model = mat4(1.0);
	Model = scale(Model, vec3(float(width), float(height), 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, vertices_cielo);
	glBindVertexArray(0);

	///////////////////////////Disegno Navicella
	glBindVertexArray(VAO);
	Model = mat4(1.0);
	Model = translate(Model, vec3(posx, posy, 0.0));
	Model = scale(Model, vec3(larghezzaGiocatore, altezzaGiocatore, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0,6);
	glBindVertexArray(0);

	//////////////////////////DISEGNO PALLA PROIETTILE
	glBindVertexArray(VAO_PALLA);
	Model = mat4(1.0);
	if (sparoInVolo) {
		Model = translate(Model, vec3(firePosition + posx_Proiettile, dimensionePalla + posy + posy_Proiettile, 0));
	}
	else {
		Model = translate(Model, vec3(posx + posx_Proiettile, dimensionePalla + posy + posy_Proiettile, 0));
	}
	Model = scale(Model, vec3(dimensionePalla, dimensionePalla, 1.0));
	glUniformMatrix4fv(MatModel1, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, triangoliPalla * 3);
	glBindVertexArray(0);


	glUseProgram(programId_1); // attiva  fragment shader_1 solo per il nemico
	glUniform1f(loc, t);

	// Disegna nemici 
	glBindVertexArray(VAO_NEMICO);
	float passo_Nemici = ((float)width) / nemici_per_riga;
	float passo_righe = 150;

	for (int i = 0; i < numeroNemici; i++) {
		if (!avversarioColpito.at(i))
		{
			Model = mat4(1.0);
			Model = translate(Model, vec3(posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i), posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i), 0));
			Model = scale(Model, vec3(dimensioneNemici, dimensioneNemici, 1.0));
			glUniformMatrix4fv(MatModel1, 1, GL_FALSE, value_ptr(Model));
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
		}
	}
	glBindVertexArray(0);

	//controllo se avversario è stato colpito da proiettile
	for (int i = 0; i < numeroNemici; i++) {
		int posizioneXavversario;
		int posizioneYavversario;
		posizioneXavversario = (posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i));
		posizioneYavversario = (posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i));

		int bound = 60;
		if (
			(
				(firePosition + posx_Proiettile + dimensionePalla / 2 >= posizioneXavversario)
				&& (firePosition + posx_Proiettile - dimensionePalla / 2 <= posizioneXavversario + dimensioneNemici))
			&&
			((posy + posy_Proiettile + dimensionePalla / 2 >= posizioneYavversario) && (posy + posy_Proiettile - dimensionePalla / 2 <= posizioneYavversario + dimensioneNemici))
			&&
			!avversarioDisattivato.at(i)
			)
		{
			if (!avversarioColpito.at(i) && !proiettileColpitoAvversario) {
				avversarioDisattivato.at(i) = true;
				proiettileColpitoAvversario = true;
				counterColpiti++;
				avversarioColpito.at(i) = true;
				printf("\ncolpito avversario: %d\n", i);

				printf("Avversari colpiti %d/%d\n", counterColpiti, numeroNemici);

				//disegno sistema particellare colpimento avversario
				attivaPS = true;
				posxPS = posizioneXavversario;
				posyPS = posizioneYavversario;
				updatePS2(0);
			}
		}
	}
	if (attivaPS) {
		glBindVertexArray(VAO_SCIA);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_SCIA);
		glBufferData(GL_ARRAY_BUFFER, nPuntiScia * sizeof(Point), &puntiScia[0], GL_STATIC_DRAW);
		// Configura l'attributo posizione
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Configura l'attributo colore
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glPointSize(7.0);
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glDrawArrays(GL_POINTS, 0, nPuntiScia);
		glBindVertexArray(0);

	}
	bool makeSwapBuffer = true;
	bool giocatoreColpito = false;
	//controllo se navicella è stata presa da un virus
	for (int i = 0; i < numeroNemici; i++) {
		int posizioneXavversario;
		int posizioneYavversario;
		posizioneXavversario = (posizioniXinizialiNemici.at(i) + sfasamentoXnemici.at(i) + direzioneXnemici.at(i));
		posizioneYavversario = (posizioniYinizialiNemici.at(i) + sfasamentoYnemici.at(i) + direzioneYnemici.at(i));

		if (
			(
				posizioneXavversario + dimensioneNemici >= posx
				&&
				posizioneXavversario <= posx + larghezzaGiocatore
				&&
				posizioneYavversario <= posy + altezzaGiocatore
				&&
				posizioneYavversario + dimensioneNemici >= posy
				)
			&&
			!avversarioDisattivato.at(i)
			)
		{
			cout << "\ngame over: sei stato colpito da un avversario\n";
			std::chrono::milliseconds timespan(3000);
			std::this_thread::sleep_for(timespan);
			makeSwapBuffer = false;
			glutDestroyWindow(window);
		}
	}
	if (!giocatoreColpito) {
		//controllo vittoria
		if (counterColpiti == numeroNemici && checkSwapBuffer < 1) {
			cout << "\nhai vinto: hai ucciso tutti gli avversari\n";
			checkSwapBuffer++;
		}

		if (checkSwapBuffer < 2 && makeSwapBuffer)
		{
			glutSwapBuffers();
		}
		else {
			std::chrono::milliseconds timespan(3000);
			std::this_thread::sleep_for(timespan);
			glutDestroyWindow(window);
		}

		if (checkSwapBuffer == 1) {
			checkSwapBuffer++;
		}
	}
}

int sanitycheck() {
	int num = 0;
	bool failed = false;
	do {
		failed = false;
		cout << "Scelta numero nemici: ";
		cin >> num;
		if (cin.fail() || num <1) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "L'input deve essere un numero maggiore di zero!" << endl;
			failed = true;
		}
	} while (failed);
	return num;
}


int main(int argc, char* argv[])
{
	numeroNemici= sanitycheck();

	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	window = glutCreateWindow("2D Game");



	glutDisplayFunc(drawScene);
	//Evento tastiera tasi premuti
	glutKeyboardFunc(keyboardPressedEvent);
	//Evento tastiera tasto rilasciato

	glutKeyboardUpFunc(keyboardReleasedEvent);
	glutTimerFunc(50, update, 0);
	glutTimerFunc(50, updateV, 0);
	glutTimerFunc(50, updateNemici, 0);
	//glutTimerFunc(50, updatePS2, 0);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();


}
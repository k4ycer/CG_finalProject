#include <Windows.h>
#include <GL/gl.h>
#include "glut.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>

using namespace std;

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;

// --------------------------------------- Variables para terreno -------------------------------- //

struct Vertex {
	float x, y, z;
	float nX, nY, nZ;
};

vector<Vertex> vertex;

float eyeX, eyeY, eyeZ, upX, upY, upZ,
lookX, lookY, lookZ, fov, midSteps,
startX, startY, stepX, stepY,
numX, numY, nearP, farP, nn, mm,
onumX, onumY, field_of_view, oeyeX,
oeyeY, oeyeZ;


// --------------------------------------- Variables para skybox -------------------------------- //
fstream inf; // global in this file for convenience
static int tamanioSkybox = 400;
static GLuint name[1];

class mRGB {
public:
	unsigned char r, g, b;

	mRGB() { r = g = b = 0; }
	mRGB(mRGB& p) { r = p.r; g = p.g; b = p.b; }
	mRGB(uchar rr, uchar gg, uchar bb) { r = rr; g = gg; b = bb; }
	void set(uchar rr, uchar gg, uchar bb) { r = rr; g = gg; b = bb; }
};

ushort getShort() //helper function
{ //BMP format uses little-endian integer types
  // get a 2-byte integer stored in little-endian form
	char ic;
	ushort ip;
	inf.get(ic); ip = ic;  //first byte is little one
	inf.get(ic);  ip |= ((ushort)ic << 8); // or in high order byte
	return ip;
}
//<<<<<<<<<<<<<<<<<<<< getLong >>>>>>>>>>>>>>>>>>>
ulong getLong() //helper function
{  //BMP format uses little-endian integer types
   // get a 4-byte integer stored in little-endian form
	ulong ip = 0;
	char ic = 0;
	unsigned char uc = ic;
	inf.get(ic); uc = ic; ip = uc;
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 8);
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 16);
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 24);
	return ip;
}

class RGBpixmap {
public:
	int nRows, nCols;
	mRGB *pixel;

	float gr, gg, gb;

	void MakeCheckerBoard();
	void MakeShadedCircle();
	void MakeRandom();
	int readBMPFile(char *fname);

	void SetTexColor(float rin, float gin, float bin);
	void SetTexture(GLuint textureName);
};

void RGBpixmap::SetTexColor(float rin, float gin, float bin) {
	// Set multipliers to set the color of the parameterized textures
	gg = gin;
	gr = rin;
	gb = bin;
}

int RGBpixmap::readBMPFile(char *fname) {  // Read into memory an mRGB image from an uncompressed BMP file.
										   // return 0 on failure, 1 on success
	inf.open(fname, ios::in | ios::binary); //read binary char's
	if (!inf) { cout << " can't open file: " << fname << endl; return 0; }
	int k, row, col, numPadBytes, nBytesInRow;
	// read the file header information
	char ch1, ch2;
	inf.get(ch1); inf.get(ch2); //type: always 'BM'
	ulong fileSize = getLong();
	ushort reserved1 = getShort();    // always 0
	ushort reserved2 = getShort();     // always 0
	ulong offBits = getLong(); // offset to image - unreliable
	ulong headerSize = getLong();     // always 40
	ulong numCols = getLong(); // number of columns in image
	ulong numRows = getLong(); // number of rows in image
	ushort planes = getShort();      // always 1
	ushort bitsPerPixel = getShort();    //8 or 24; allow 24 here
	ulong compression = getLong();      // must be 0 for uncompressed
	ulong imageSize = getLong();       // total bytes in image
	ulong xPels = getLong();    // always 0
	ulong yPels = getLong();    // always 0
	ulong numLUTentries = getLong();    // 256 for 8 bit, otherwise 0
	ulong impColors = getLong();       // always 0
	if (bitsPerPixel != 24)
	{ // error - must be a 24 bit uncompressed image
		cout << "not a 24 bit/pixelimage, or is compressed!\n";
		inf.close(); return 0;
	}
	//add bytes at end of each row so total # is a multiple of 4
	// round up 3*numCols to next mult. of 4
	nBytesInRow = ((3 * numCols + 3) / 4) * 4;
	numPadBytes = nBytesInRow - 3 * numCols; // need this many
	nRows = numRows; // set class's data members
	nCols = numCols;
	pixel = new mRGB[nRows * nCols]; //make space for array
	if (!pixel) return 0; // out of memory!
	long count = 0;
	char dum;
	for (row = 0; row < nRows; row++) // read pixel values
	{
		for (col = 0; col < nCols; col++)
		{
			char r, g, b;
			inf.get(b); inf.get(g); inf.get(r); //read bytes
			pixel[count].r = r; //place them in colors
			pixel[count].g = g;
			pixel[count++].b = b;
		}
		for (k = 0; k < numPadBytes; k++) //skip pad bytes at row's end
			inf >> dum;
	}
	inf.close(); return 1; // success
}

void RGBpixmap::SetTexture(GLuint textureName) {
	// TO DO
	// Bind 2D textures, based on input name
	glBindTexture(GL_TEXTURE_2D, textureName);
	// Set texture parameters to initial values
	// Set wrapping and interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate 2D texture image - use class member variables: nCols, nRows, pixel
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nCols, nRows, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
}

RGBpixmap skyboxTex;


// ---------------------------------------- Variables para avioneta -------------------------------//

// Global variables for each of the your angles
static float LA1 = 0.0; //LA1
static float LB1 = 0.0; // LB1
static float LA2 = 0.0; // LA2
static float LB2 = 0.0; // LB2
static float LA3 = 0.0; //LA3
static float LB3 = 0.0; //LB3
static float LA0 = 0.0; //LA0
static float LB0 = 0.0; //LB0
static float angleInc = 3.0;
float hight = 5.0;

typedef struct treenode
{
	//GLfloat m[16];
	void(*transform)();
	void(*draw)();
	struct treenode *sibling;
	struct treenode *child;
} treenode;

treenode cuerpo, cabina, pico, ala1, ala2, ala3, ala4, fuga2, fuga4, llanta1, llanta2, llanta3, rueda1, rueda2, rueda3, alat1, alat2, bandera;

void traverse(treenode *root)
{
	// Check to see if this node is null (error checking)
	//if(root == NULL)
	//	return;

	// Apply transformation for current node and draw
	// Continue with depth-first search (go to children)
	glPushMatrix();

	root->transform();
	root->draw();

	// Check for children
	// NOTE that transformation is still applied
	if (root->child != NULL)
		traverse(root->child);

	glPopMatrix();

	// Check for siblings
	if (root->sibling != NULL)
		traverse(root->sibling);
}

// --------------------------------------- Funciones para skybox -------------------------------- //
void Skybox() {
	glEnable(GL_TEXTURE_2D);

	// Set texture mode
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	// Bind texture
	glBindTexture(GL_TEXTURE_2D, name[0]);

	float mitad = tamanioSkybox / 2;

	glEnable(GL_TEXTURE_2D);

	// Izquierda

	glBegin(GL_QUADS);
	glNormal3d(1, 0, 0);

	glTexCoord2d(0.0, 0.33333);
	glVertex3f(-mitad, mitad, mitad);

	glTexCoord2d(0.25, 0.33333);
	glVertex3f(-mitad, mitad, -mitad);

	glTexCoord2d(0.25, 0.66666);
	glVertex3f(-mitad, -mitad, -mitad);

	glTexCoord2d(0.0, 0.66666);
	glVertex3f(-mitad, -mitad, mitad);
	glEnd();


	// En frente

	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);

	glTexCoord2d(0.25, 0.33333);
	glVertex3f(-mitad, mitad, -mitad);

	glTexCoord2d(0.5, 0.33333);
	glVertex3f(mitad, mitad, -mitad);

	glTexCoord2d(0.5, 0.66666);
	glVertex3f(mitad, -mitad, -mitad);

	glTexCoord2d(0.25, 0.66666);
	glVertex3f(-mitad, -mitad, -mitad);

	glEnd();


	// Derecha
	glBegin(GL_QUADS);
	glNormal3d(-1, 0, 0);

	glTexCoord2d(0.5, 0.33333);
	glVertex3f(mitad, mitad, -mitad);

	glTexCoord2d(0.75, 0.33333);
	glVertex3f(mitad, mitad, mitad);

	glTexCoord2d(0.75, 0.66666);
	glVertex3f(mitad, -mitad, mitad);

	glTexCoord2d(0.5, 0.66666);
	glVertex3f(mitad, -mitad, -mitad);
	glEnd();


	// Atras
	glBegin(GL_QUADS);
	glNormal3d(0, 0, -1);

	glTexCoord2d(0.75, 0.33333);
	glVertex3f(mitad, mitad, mitad);

	glTexCoord2d(1, 0.33333);
	glVertex3f(-mitad, mitad, mitad);

	glTexCoord2d(1, 0.66666);
	glVertex3f(-mitad, -mitad, mitad);

	glTexCoord2d(0.75, 0.66666);
	glVertex3f(mitad, -mitad, mitad);
	glEnd();


	// Techo
	glBegin(GL_QUADS);
	glNormal3d(0, -1, 0);

	glTexCoord2d(0.25, 0.0);
	glVertex3f(-mitad, mitad, mitad);

	glTexCoord2d(0.5, 0);
	glVertex3f(mitad, mitad, mitad);

	glTexCoord2d(0.5, .3333);
	glVertex3f(mitad, mitad, -mitad);

	glTexCoord2d(0.25, .3333);
	glVertex3f(-mitad, mitad, -mitad);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


// -----------------------------------------Funciones para crear la avioneta ---------------------//


void glutSolidCylinder(float r, float h, int n, int m)
{
	glPushMatrix();
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0F, 0.0F, -h / 2);
	GLUquadricObj * qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluCylinder(qobj, r, r, h, n, m);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}

void RenderCuerpo()
{
	glPushMatrix();
	glScaled(2.0, 2.0, hight * 2);
	glutSolidSphere(1.0, 15, 15);
	glPopMatrix();
}

void RenderPico()
{
	glPushMatrix();
	glTranslated(0.0, 0.0, hight * 2 - 1.0);
	glScaled(0.5, 0.5, 1.5);
	glutSolidSphere(1.0, 15, 15);
	glPopMatrix();
}

void RenderCabina()
{
	glPushMatrix();
	glTranslated(0.0, 1.0, hight - 2.0);
	glScaled(1.5, 1.5, hight);
	glutSolidSphere(1.0, 15, 15);
	glPopMatrix();
}

void TransformNothign()
{

}

void RenderAla1()
{
	glPushMatrix();
	glTranslated(hight / 2 + 1, 0.0, hight - 2.0);
	glScaled(hight + 1, 0.1, hight / 2 + 1.0);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderAla3()
{
	glPushMatrix();
	//glTranslated(hight*1.25, 0.0, hight - 2.0);
	glScaled(hight / 2, 0.1, hight / 2);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderAla2()
{
	glPushMatrix();
	glTranslated(-hight + 1, 0.0, hight - 2.0);
	glScaled(hight + 1, 0.1, hight / 2 + 1.0);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderAla4()
{
	glPushMatrix();
	glScaled(hight / 2, 0.1, hight / 2);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderFuga4()
{
	glPushMatrix();
	glTranslated(-hight, -0.75, hight - 2.0);
	glRotated(90, 1.0, 0.0, 0.0);
	glutSolidCylinder(0.7, hight / 2, 15, 55);
	glPopMatrix();
}

void TransformAla3()
{
	glTranslated(hight*1.5, 0.0, hight - 2.0);
	glRotated(LA3, 1.0, 0.0, 0.0);
}

void TransformAla4()
{
	glTranslated(-hight*1.65, 0.0, hight - 2.0);
	glRotated(LA3, 1.0, 0.0, 0.0);
}

void RenderFuga2()
{
	glPushMatrix();
	glTranslated(hight / 2 + hight / 4 + 1, -0.75, hight - 2.0);
	glRotated(90, 1.0, 0.0, 0.0);
	glutSolidCylinder(0.7, hight / 2, 15, 55);
	glPopMatrix();
}

void RenderBandera()
{
	glPushMatrix();
	glTranslated(0.0, 2.0, -hight*1.5);
	glRotated(90, 0.0, 0.0, 1.0);
	glScaled(hight / 2, 0.1, hight / 3);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderAlat1()
{
	glPushMatrix();
	glTranslated(hight / 2, 0.0, -hight);
	glScaled(hight / 2, 0.1, hight / 2);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderAlat2()
{
	glPushMatrix();
	glTranslated(-hight + 2.5, 0.0, -hight);
	glScaled(hight / 2, 0.1, hight / 2);
	glutSolidCube(1.0);
	glPopMatrix();
}

void RenderLlantas()
{
	glPushMatrix();
	glutSolidCylinder(0.2, hight / 2, 15, 55);
	glPopMatrix();
}

void RenderRuedas()
{
	glPushMatrix();
	glRotated(90, 0.0, 1.0, 0.0);
	glutSolidTorus(0.2, 0.4, 15, 15);
	glPopMatrix();
}

void TransformLlanta1()
{
	glTranslated(hight / 4, -1.0, -hight);
	glRotated(LB3, 1.0, 0.0, 0.0);
}

void TransformLlanta2()
{
	glTranslated(-hight / 4, -1.0, -hight);
	glRotated(LB3, 1.0, 0.0, 0.0);
}

void TransformLlanta3()
{
	glTranslated(0.0, -1.0, hight*1.5);
	glRotated(LB3, 1.0, 0.0, 0.0);
}

void TransformRuedas()
{
	glTranslated(0.0, -1.6, 0.0);
	glRotated(LA0, 1.0, 0.0, 0.0);
}

void CreateInsect()
{

	cuerpo.draw = RenderCuerpo;
	cuerpo.transform = TransformNothign;
	cuerpo.child = &cabina;
	cuerpo.sibling = &pico;

	pico.draw = RenderPico;
	pico.transform = TransformNothign;
	pico.child = NULL;
	pico.sibling = NULL;

	cabina.draw = RenderCabina;
	cabina.transform = TransformNothign;
	cabina.child = NULL;
	cabina.sibling = &ala1;

	ala1.draw = RenderAla1;
	ala1.transform = TransformNothign;
	ala1.child = &fuga2;
	ala1.sibling = &ala3;

	fuga2.draw = RenderFuga2;
	fuga2.transform = TransformNothign;
	fuga2.child = NULL;
	fuga2.sibling = NULL;

	ala3.draw = RenderAla3;
	ala3.transform = TransformAla3;
	ala3.child = NULL;
	ala3.sibling = &ala2;

	ala2.draw = RenderAla2;
	ala2.transform = TransformNothign;
	ala2.child = &fuga4;
	ala2.sibling = &ala4;

	fuga4.draw = RenderFuga4;
	fuga4.transform = TransformNothign;
	fuga4.child = NULL;
	fuga4.sibling = NULL;

	ala4.draw = RenderAla4;
	ala4.transform = TransformAla4;
	ala4.child = NULL;
	ala4.sibling = &bandera;

	bandera.draw = RenderBandera;
	bandera.transform = TransformNothign;
	bandera.child = &alat1;
	bandera.sibling = &llanta1;

	alat1.draw = RenderAlat1;
	alat1.transform = TransformNothign;
	alat1.child = NULL;
	alat1.sibling = &alat2;

	alat2.draw = RenderAlat2;
	alat2.transform = TransformNothign;
	alat2.child = NULL;
	alat2.sibling = NULL;

	llanta1.draw = RenderLlantas;
	llanta1.transform = TransformLlanta1;
	llanta1.child = &rueda1;
	llanta1.sibling = &llanta2;

	llanta2.draw = RenderLlantas;
	llanta2.transform = TransformLlanta2;
	llanta2.child = &rueda2;
	llanta2.sibling = &llanta3;

	llanta3.draw = RenderLlantas;
	llanta3.transform = TransformLlanta3;
	llanta3.child = &rueda3;
	llanta3.sibling = NULL;

	rueda1.draw = RenderRuedas;
	rueda1.transform = TransformRuedas;
	rueda1.child = NULL;
	rueda1.sibling = NULL;

	rueda2.draw = RenderRuedas;
	rueda2.transform = TransformRuedas;
	rueda2.child = NULL;
	rueda2.sibling = NULL;

	rueda3.draw = RenderRuedas;
	rueda3.transform = TransformRuedas;
	rueda3.child = NULL;
	rueda3.sibling = NULL;


}

void setMaterialAvioneta()
{
	// Define material properties
	GLfloat mat_specular[] = { 0.775, 0.775, 0.775, 1.0 };
	GLfloat mat_shininess[] = { 76.0 };
	GLfloat mat_surface[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat diff_surface[] = { 0.4, 0.4, 0.4, 1.0 };

	// Set material properties, as defined above
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_surface);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_surface);

	//Set shading model to use
	glShadeModel(GL_SMOOTH);
	//Enable depth testing (for hidden surface removal)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LESS);
}
// ----------------------------------------- Funciones para crear el terreno ---------------------//

//"findNomrla" es usada para encontrar la normal entre dos vertices utilizando el producto cruz
struct Vertex findNormal(struct Vertex p1, struct Vertex p2, struct Vertex p3) {

	//Usando los puntos p1, p2 y p3 encontramos los dos vectores
	struct Vertex vector1 = { (p2.x - p1.x),(p2.y - p1.y),(p2.z - p1.z) };
	struct Vertex vector2 = { (p3.x - p1.x),(p3.y - p1.y),(p3.z - p1.z) };
	
	//Haciendo uso del producto punto calculamos la normal 
	float xComp = (vector1.y*vector2.z) - (vector1.z*vector2.y);
	float yComp = (vector1.z*vector2.x) - (vector1.x*vector2.z);
	float zComp = (vector1.x*vector2.y) - (vector1.y*vector2.x);
	struct Vertex normal = { xComp, yComp, zComp };
	return normal;
}

//"encontrarNorTerr" nos ayuda a encontrar las normales del terreno
void encontrarNorTerr()
{
	for (int i = 0; i < (numY - 1); i++) {

		for (int j = 0; j < (numX - 1); j++) {

			if (i == 0 && j == 0) {


				Vertex normal = findNormal(vertex.at(i*numX + j), vertex.at((i + 1)*numX + j), vertex.at(i*numX + (j + 1)));
				vertex.at(i*numX + j).nX = normal.x;
				vertex.at(i*numX + j).nY = normal.y;
				vertex.at(i*numX + j).nZ = normal.z;

			}
			else if (i == 0) {
				Vertex normal1 = findNormal(vertex.at(i*numX + j - 1), vertex.at((i + 1)*numX + j - 1), vertex.at(i*numX + j));
				Vertex normal2 = findNormal(vertex.at((i + 1)*numX + j - 1), vertex.at((i + 1)*numX + j), vertex.at(i*numX + j));
				Vertex normal3 = findNormal(vertex.at(i*numX + j), vertex.at((i + 1)*numX + j), vertex.at(i*numX + j + 1));

				vertex.at(i*numX + j).nX = (normal1.x + normal2.x + normal3.x) / 3;
				vertex.at(i*numX + j).nY = (normal1.y + normal2.y + normal3.y) / 3;
				vertex.at(i*numX + j).nZ = (normal1.z + normal2.z + normal3.z) / 3;
			}
			else if (j == 0) {
				Vertex normal1 = findNormal(vertex.at((i - 1)*numX + j), vertex.at(i*numX + j), vertex.at((i - 1)*numX + j + 1));
				Vertex normal2 = findNormal(vertex.at((i - 1)*numX + j + 1), vertex.at(i*numX + j), vertex.at(i*numX + j + 1));
				Vertex normal3 = findNormal(vertex.at(i*numX + j), vertex.at((i + 1)*numX + j), vertex.at(i*numX + j + 1));

				vertex.at(i*numX + j).nX = (normal1.x + normal2.x + normal3.x) / 3;
				vertex.at(i*numX + j).nY = (normal1.y + normal2.y + normal3.y) / 3;
				vertex.at(i*numX + j).nZ = (normal1.z + normal2.z + normal3.z) / 3;
			}
			else {
				Vertex normal1 = findNormal(vertex.at((i - 1)*numX + j), vertex.at(i*numX + j - 1), vertex.at(i*numX + j));
				Vertex normal2 = findNormal(vertex.at((i + 1)*numX + j - 1), vertex.at(i*numX + j - 1), vertex.at(i*numX + j));
				Vertex normal3 = findNormal(vertex.at((i + 1)*numX + j - 1), vertex.at((i + 1)*numX + j), vertex.at(i*numX + j));
				Vertex normal4 = findNormal(vertex.at(i*numX + j + 1), vertex.at((i + 1)*numX + j), vertex.at(i*numX + j));
				Vertex normal5 = findNormal(vertex.at((i - 1)*numX + j + 1), vertex.at(i*numX + j + 1), vertex.at(i*numX + j));
				Vertex normal6 = findNormal(vertex.at((i - 1)*numX + j), vertex.at((i - 1)*numX + j + 1), vertex.at(i*numX + j + 1));

				vertex.at(i*numX + j).nX = (normal1.x + normal2.x + normal3.x + normal4.x + normal5.x + normal6.x) / 6;
				vertex.at(i*numX + j).nY = (normal1.y + normal2.y + normal3.y + normal4.y + normal5.y + normal6.y) / 6;
				vertex.at(i*numX + j).nZ = (normal1.z + normal2.z + normal3.z + normal4.z + normal5.z + normal6.z) / 6;
			}
		}
	}

}

//"dibujarTerreno" con ayuda de GL_TRIANGLES dibujamos la maya que usara el terreno
void dibujarTerreno()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < (numY -1); i++) {
		for (int j = 0; j < (numX -1); j++) {

			int index = (i*numX + j);
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

			index = ((i + 1)*numX + j);
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

			index = (i*numX + (j + 1));
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

			index = (i*numX + (j + 1));
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

			index = ((i + 1)*numX + (j + 1));
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

			index = ((i + 1)*numX + j);
			glNormal3f(vertex.at(index).nX, vertex.at(index).nY, vertex.at(index).nZ);
			glVertex3f(vertex.at(index).x, vertex.at(index).y, vertex.at(index).z);

		}
	}
	glEnd();
}

//Leemos el archivo que contiene el Height Map para crear "montañas" dentro de la maya. El archivo contiene
//Otros parametros como la posición de la camara y el punto a donde ve la misma
int leerArchivo()
{
	int in;
	ifstream readFile;
	readFile.open("parametros.txt");

	if (!readFile) {
		cout << "No es posible abrir el archivo" << endl;
		return 1;
	}
	readFile >> eyeX;
	readFile >> eyeY;
	readFile >> eyeZ;
	readFile >> upX;
	readFile >> upY;
	readFile >> upZ;
	readFile >> lookX;
	readFile >> lookY;
	readFile >> lookZ;
	readFile >> fov;
	readFile >> nearP;
	readFile >> farP;
	readFile >> midSteps;
	readFile >> startX;
	readFile >> stepX;
	readFile >> numX;
	readFile >> startY;
	readFile >> stepY;
	readFile >> numY;

	onumX = numX;
	onumY = numY;
	oeyeX = eyeX;
	oeyeY = eyeY;
	oeyeZ = eyeZ;
	field_of_view = fov;

	for (int i = 0; i < numY; i++) {
		for (int j = 0; j < numX; j++) {
			Vertex v;
			v.x = startX + (j*stepX);
			v.y = startY + (i*stepY);
			readFile >> v.z;
			vertex.push_back(v);
		}
	}
	readFile.close();
}

void asigMater()
{
	GLfloat ambient[4] = { 0.7, 0.2, 0.4, 1.0 };    
	GLfloat specular[4] = { 0.7, 0.8, 0.1, 1.0 };   
	GLfloat diffuse[4] = { 0.7, 0.8, 0.1, 1.0 };  
	
	//Color del "cielo"
	glClearColor(0.09, 0.61, 0.85, 0.0);



	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0);

	//Set shading model to use
	glShadeModel(GL_SMOOTH);
	//Enable depth testing (for hidden surface removal)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LESS);

}

void asigLuces()
{

	glShadeModel(GL_SMOOTH);

	GLfloat lightP[4] = { 00.0, 0.9, 00.0,1.0 };
	GLfloat lightA[4] = { 0.0,0.9,0.4,1.0 };
	GLfloat lightS[4] = { 0.0,0.9,0.4,1.0 };
	GLfloat lightD[4] = { 0.9,0.9,0.9,1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightP);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightA);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightS);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightD);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(field_of_view, 1.0, nearP, farP * 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(oeyeX, oeyeY, oeyeZ, lookX, lookY, lookZ, upX, upY, upZ);

	// Dibujar skybox
	glPushMatrix();
	glRotated(270, 1, 0, 0);
	glScaled(3.2, 3.2, 3.2);
	Skybox();
	glPopMatrix();


	//Se asignan los materiales

	//static float r = 0;
	//r += 0.1;
	glPushMatrix();
	//glRotatef(r, 0, 0, 1);
	glTranslated(-(onumX*stepX) / 2, -(onumY*stepY) / 2, 0);

	glPushMatrix();
    glTranslated(840, 890, 70);
	glRotated(90, 1, 0, 0);
	setMaterialAvioneta();
	traverse(&cuerpo);
	glPopMatrix();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	asigMater();
	encontrarNorTerr();
	dibujarTerreno();
	glPopMatrix();
	glutSwapBuffers();
	glutPostRedisplay();
}

void processSpecialKeys(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP:
		oeyeX -= 1;
		break;
	case GLUT_KEY_DOWN:
		oeyeX += 1;
		break;
	case GLUT_KEY_LEFT:
		oeyeY -= 1;
		break;
	case GLUT_KEY_RIGHT:
		oeyeY += 1;
		break;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:		
		exit(0);
		break;
	case 'r':			
		field_of_view = fov;
		oeyeX = eyeX;
		oeyeY = eyeY;
		oeyeZ = eyeZ;
		break;
	}

	glutPostRedisplay();
}
void init(void)
{
	CreateInsect();
}



int main(int argc, char** argv)
{
	// Create tree structure
	init();
	leerArchivo();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Project terrain");

	// Get OpenGL to automatically generate the texture "names"
	glGenTextures(1, name);

	// Texture skybox
	skyboxTex.SetTexColor(1.0, 1.0, 1.0);
	skyboxTex.readBMPFile("C:/Users/Eduardo Torres/Documents/Eduardo/Graficas/finalProject/finalProject/skybox2.bmp");
	skyboxTex.SetTexture(name[0]);

	//Asignar las luces
	asigLuces();

	glEnable(GL_NORMALIZE);
	
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	//midPointAl();
	glutSpecialFunc(processSpecialKeys);
	glutMainLoop();
	return 0;
}
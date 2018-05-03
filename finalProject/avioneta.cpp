#include "avioneta.h"
#include <GL/glut.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

// Global variables for each of the your angles
static float LA1 = 0.0; //LA1
static float LB1 = 0.0; // LB1
static float LA2 = 0.0; // LA2
static float LB2 = 0.0; // LB2
static float LA3 = 0.0; //LA3
static float LB3 = 0.0; //LB3
static float LA0 = 0.0; //LA0
static float LB0 = 0.0; //LB0

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

 void avioneta::traverse(treenode *root)
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

void avioneta::RenderCuerpo()
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
	cuerpo.draw = avioneta::RenderCuerpo;
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
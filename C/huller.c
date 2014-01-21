#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
/*
Wichtige Werte für berechnung:
Xp  (Vektor)
Xn  (Vektor)
XpXp (Skalar)
XnXp (Skalar)
XnXn (Skalar)
*/

typedef struct{
	float* coords;
	int dim;
    int class;
} point;

//Wichtige Daten für Huller
typedef struct{
   point *Xp;
   point *Xn;
   float XpXp;
   float XnXp;
   float XnXn;
} huller;

point *createPoint(int dim);
void destroyPoint(point *p);
float dotP(point *p1,point *p2);
point *randomPoint(int dim);
void printPoint(point *p);
point *avgPoints(point *p1, point *p2);
void test();

int main(int argc, char **argv){
    test();
}

//punkt im dim-dimensionalen raum erstellen
point *createPoint(int dim){
	assert(dim>0);
	point *p=malloc(sizeof(point));	//speicher für struct reservieren
	p->dim=dim;
	p->coords=malloc(sizeof(float)*dim); //speicher für float-array
    p->class=0;
	return p;
}


//punkt freigeben
void destroyPoint(point *p){
	free(p->coords);
	free(p);
}

//skalarprodukt
float dotP(point *p1, point *p2){
	assert(p1->dim == p2->dim);
	float ret=0.0;   //zwangsweise initialisieren?	
	for(int i=0;i<p1->dim;i++){
		ret+=(p1->coords[i])*(p2->coords[i]);	
	}	
	return ret;
}

//zufälligen punkt erstellen zum testen
point *randomPoint(int dim){
	point *p=createPoint(dim);
	for(int i=0;i<dim;i++){
		p->coords[i]=(float)random();
	}
	return p;
}


//punkt ausgeben zum testen
void printPoint(point *p){
	printf("Dimension: %d\n",(int)p->dim);
	printf("<");
	for(int i=0; i < p->dim; i++){
		printf("%.2f",p->coords[i]);
		if(i!=(p->dim)-1){
			printf(",");
		}
	}
	printf(">\n");
}

//durchschnitt zwischen 2 punkten errechnen
point *avgPoints(point *p1, point *p2){
	assert(p1->dim==p2->dim);	
	point *p=createPoint(p1->dim);
	for(int i=0;i<p1->dim;i++){
		p->coords[i]=((p1->coords[i])+(p2->coords[i]))/2;
	}
    return p;
}


//Konstruktor für Huller
huller *createHuller(int dim){
    huller *h = malloc(sizeof(huller));
    h->Xp = createPoint(dim);
    h->Xn = createPoint(dim);
    h->XpXp = 0.0;
    h->XnXp = 0.0;
    h->XnXn = 0.0;
    return h;
}


//Destruktor
void destroyHuller(huller *h){
    destroyPoint(h->Xp);
    destroyPoint(h->Xn);
    free(h);
}

//ein paar dinge testen
void test(){
	point *p1 = createPoint(3);
	p1->coords[0]=0;
	p1->coords[1]=1;
	p1->coords[2]=2;
	point *p2 = createPoint(3);
	p2->coords[0]=4;
	p2->coords[1]=2;
	p2->coords[2]=5;
	printPoint(p1);
	printPoint(p2);
	printf("Skalarprodukt p1,p2 = %f\n",dotP(p1,p2));
    printf("Avg p1,p2: \n");
    point *p3 = avgPoints(p1,p2);
    printPoint(p3);
    printf("\n");
	destroyPoint(p1);
	destroyPoint(p2);
    destroyPoint(p3);
    p1 = randomPoint(10);
    p2 = randomPoint(10);
    printPoint(p1);
    printPoint(p2);
    p3 = avgPoints(p1,p2);
    printPoint(p3);
    destroyPoint(p1);
    destroyPoint(p2);
    destroyPoint(p3);
}


// Was passiert, wenn nach dem Erstellen der Ebene ein Wert genau auf der ebene landet? geht das überhaupt? Wie wird dieser klassifiziert?
// Was ist der a-(alpha)-Wert ?
// Für was benötigen wir Lambda?
// Wird zur Vereinfachung auch hier der bias-Wert weggelassen?
// -- Einlesen der Dateien -- ?

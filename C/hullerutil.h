#include <stdio.h>
#include "point.h"
//Array aus count vielen Punkten
typedef struct{
    int count_p;
    int count_n;
    point **sample_p;  //ein array aus zeigern auf punkte (?)
    point **sample_n; //negative beispiele
} samples;

//Wichtige Daten für Huller
typedef struct{
   point *Xp;
   point *Xn;
   double XpXp;
   double XnXp;
   double XnXn;
} huller;

samples* createSamples();
void sampleAdd(samples* s,point* p);
void readSamples(char *file,int dim,samples *s);
void destroySamples(samples* s);
void destroyHuller(huller *h);
huller *createHuller(int dim); 
huller *hullerFromFile(char* file);
void updateScalars(huller *h);
point* randPoint(samples* s);
void printSamples(samples* s);
void printHuller(huller *h);
void alphaStats(samples *s);
int alphaNotNull(samples *s);

#include <stdio.h>
#include "point.h"
#include "alphalist.h"
//Array aus count vielen Punkten
typedef struct{
    int count_p;
    int count_n;
    point **sample_p;  //ein array aus zeigern auf punkte (?)
    point **sample_n; //negative beispiele
    alphalist *p;   //alphaliste der negativen punkte
    alphalist *n;   //alphaliste der positiven punkte
} samples;

typedef struct{
    int n;
    double *alphas;
} alphacompare;

//Wichtige Daten für Huller
typedef struct{
   point *Xp;
   point *Xn;
   double XpXp;
   double XnXp;
   double XnXn;
} huller;

alphacompare *createAlphac(samples* s);
void destroyAlphac(alphacompare* a);
samples* createSamples();
void sampleAdd(samples* s,point* p);
void readSamples(char *file,int dim,samples *s);
void destroySamples(samples* s);
void destroyHuller(huller *h);
huller *createHuller(int dim); 
huller *hullerFromFile(char* file);
void updateScalars(huller *h);
point* randPoint(samples* s,int alpha);
void printSamples(samples* s);
void printHuller(huller *h);
void alphaStats(samples *s);
int alphaNotNull(samples *s);

#include <stdio.h>
#include <stdlib.h>

//doppelnennungen können nicht in einer alphaliste auftreten
//=> Müssen daher nicht abgefangen werden.

/*
Besteht aus 2 Listen:
alphas_n beinhaltet die indizes der punkte, deren alpha=0 ist
alphas_nn -"- , deren alpha!=0 ist
*/
typedef struct{
    int c_null;
    int c_notnull;
    int *alphas_n;  //indizes von punkten null
    int *alphas_nn; //indizes von punkten nicht null
} alphalist;
//Konstruktor
alphalist *createAlphalist();
//destruktor
void destroyAlphalist(alphalist *a);
//Alpha hinzufügen
void addAlpha(alphalist *a,int index, double alpha);
//element in o(1) aus liste löschen
void removeAlpha(alphalist *a,int i,double alpha);
//Alpha von punkt hat sich geändert (das wissen wir)
void switchPoint(alphalist *a,int index, double alpha);

void printAlphalist(alphalist *a);

void testAlphalist();

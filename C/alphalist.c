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
alphalist *createAlphalist(){
    alphalist *a = malloc(sizeof(alphalist));
    a->alphas_n = calloc(0,sizeof(int));
    a->alphas_nn = calloc(0,sizeof(int));
    a->c_null=0;
    a->c_notnull=0;
    return a;
}
//destruktor
void destroyAlphalist(alphalist *a){
    free(a->alphas_n);
    free(a->alphas_nn);
    free(a);
}
//Alpha hinzufügen
void addAlpha(alphalist *a,int index, double alpha){
    if(alpha==0.0){
        a->c_null=a->c_null+1;
        a->alphas_n=realloc(a->alphas_n,a->c_null*sizeof(int));
        a->alphas_n[a->c_null-1]=index;
    }else{ 
        a->c_notnull=a->c_notnull+1;
        a->alphas_nn=realloc(a->alphas_nn,a->c_notnull*sizeof(int));
        a->alphas_nn[a->c_notnull-1]=index;
    }
}
//element in o(1) aus liste löschen
void removeAlpha(alphalist *a,int i,double alpha){
    if(alpha==0.0){ //es muss aus notnull gelöscht werden
        a->alphas_nn[i]=a->alphas_nn[a->c_notnull-1]; //letztes auf pos 1 speichern
        a->c_notnull=a->c_notnull-1; //länge verringert sich um 1
        a->alphas_nn=realloc(a->alphas_nn,a->c_notnull*sizeof(int)); //reallozieren 
    }else{  //muss aus null gelöscht werden
        a->alphas_n[i]=a->alphas_n[a->c_null-1];
        a->c_null=a->c_null-1;
        a->alphas_n=realloc(a->alphas_n,a->c_null*sizeof(int));
    }
}
//Alpha von punkt hat sich geändert (das wissen wir)
void switchPoint(alphalist *a,int index, double alpha){
    if(alpha==0.0){ //alpha vom punkt 0 geworden
        //index aus alpha!=0 löschen
        for(int i=0;i<a->c_notnull;i++){    //liste durchsuchen
            if(a->alphas_nn[i]==index){     //index gefunden
                removeAlpha(a,i,alpha); //an stelle i löschen
                addAlpha(a,index,alpha); //index an andere liste hängen
                break;  //suche beenden, wir sind fertig.
            }
        }
    }else{ //alpha vom punkt ist !=0 geworden
        for(int i=0;i<a->c_null;i++){ //in liste alpha=0 suchen
            if(a->alphas_n[i]==index){
                removeAlpha(a,i,alpha); //an stelle i löschen aus alpha=0
                addAlpha(a,index,alpha);    //hinten an alpha!=0 hängen.
                break;
            }
        }
    }
}

void printAlphalist(alphalist *a){
    fprintf(stderr,"Alpha=0 size=%d\n",a->c_null);
    for(int i=0;i<a->c_null;i++){
        fprintf(stderr,"%d\n",a->alphas_n[i]);
    }
    fprintf(stderr,"Alpha!=0 size=%d\n",a->c_notnull);
    for(int i=0;i<a->c_notnull;i++){
        fprintf(stderr,"%d\n",a->alphas_nn[i]);
    }
}

void testAlphalist(){
    alphalist* a=createAlphalist();
    for(int i=0;i<10;i++){   //hinzufügen testen
        addAlpha(a,i,0.0);
    }
    for(int i=0;i<10;i++){
        addAlpha(a,i,1.0);
    }
    printAlphalist(a);  //ausgeben
    switchPoint(a,0,0.0); //index 0,1,2,3,4 in alpha_n verschieben
    printAlphalist(a);
    switchPoint(a,1,0.0);
    printAlphalist(a);
    switchPoint(a,2,0.0);
    printAlphalist(a);
    switchPoint(a,3,0.0);
    printAlphalist(a);
    switchPoint(a,4,0.0);
    printAlphalist(a);
    destroyAlphalist(a);
}

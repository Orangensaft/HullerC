#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
//#include "point.h"
//#include "hullerutil.h"
#include "hullerutil.h"
//Konstanten
//Anzahl der Punkte die für die Initialisierung genutzt werden
#define AVGCOUNT 500
//Maximale Anzahl der Iterationen
#define MAXITERATIONS 1000000
// je größer desto genauer
#define CONVERGE 100
//anzahl alpha!=0 zum abbrechen

int debug=0;

//tests ANFANG
void testPoint();
void testFile(char *input,int dim);
void testAddComp();
void sampleTest();
void testHINIT();
void loadHullerTest();
void testInit();
void completeTest();
//Tests ENDE

void initHuller(huller* h,samples* s);
void mainHuller(huller* h, samples* s);
void updateHuller(huller* h, samples* s,point* xn);
double max(double a, double b);
double min(double a, double b);
void classify(char* svmfile, char* hulfile,int dim);
void learn(char* svmfile,int dim);




int main(int argc, char **argv){
    srandom((int)time(NULL));
    if(argc==1){
        debug=1;
        fprintf(stderr,"Keine Argumente angegeben; Wechsele in Testmodus\n");/*
        getchar();
        loadHullerTest();
        testHINIT();
        testPoint();
        sampleTest();    
        testFile("a1a.svm",124);
        testAddComp();   
        testInit();
        completeTest();    
        */    
        exit(EXIT_SUCCESS);
    }  
    if(argc==4){
            if(strcmp(argv[1],"learn")==0){
                learn(argv[2],atoi(argv[3])); // argv[2] = SVM-File ; argv[3] = Dimension
                exit(EXIT_SUCCESS); 
            }
    }
    if(argc==5){
            if(strcmp(argv[1],"classify")==0){
                classify(argv[2],argv[3],atoi(argv[4])); // argv[2] = SVM-File ; argv[3] = unser Modell ; argv[4] = Dimension
                exit(EXIT_SUCCESS);
            }
    }
    printf("Anwendung: huller learn SVM-File #attribute\n");
    printf("Oder:      huller classify SVM-File hullerfile #attribute\n");
    exit(EXIT_SUCCESS);    
}

void learn(char* svmfile, int dim){
    huller *h=createHuller(dim);
    samples *s=createSamples();
    readSamples(svmfile,dim,s);
    fprintf(stderr,"Samples eingelesen. Starte Huller\n");
    mainHuller(h,s);
    fprintf(stderr,"Huller fertig.\n");
    printHuller(h);
    destroyHuller(h);
    destroySamples(s);
}

void classify(char* svmfile, char* hulfile,int dim){
    huller *h=hullerFromFile(hulfile); //hullermodell einlesen
    double rate=0.0;
    samples *s=createSamples();
    readSamples(svmfile,dim,s);
    point *tmp=createPoint(dim);
    point *n=createPoint(dim);
    pointAdd(n,h->Xp); // n auf Xp setzen
    pointSub(n,h->Xn); //->(Xp-Xn)   -> Normalenvektor n
    point *a=createPoint(dim);
    pointAdd(a,h->Xp);
    avgPoints(a,h->Xn); // a -> Ortsvektor (Xp+Xn)/2
    int classold=0;
    int class=0;
    double yx=0.0;
    //y(x)=dotP(x-(Xp+Xn)/2,tmp)
    fprintf(stderr,"Klassifiziere 'positive Punkte'\n");
    for(int i=0;i<s->count_p;i++){ //zunächst 'positive' klassifizieren
        classold=s->sample_p[i]->class;
        pointSet(tmp,0.0);          //tmp auf 0 setzen
        pointAdd(tmp,s->sample_p[i]);  //tmp= x
        pointSub(tmp,a);            //tmp = x-a
        yx=dotP(tmp,n);     //yx ) (x-a)*n
        if(yx>=0){  //eine seite ebene
            class=1;
        }else{      //andere seite
            class=0;
        }
        if(classold==class){ //richtig! treffer zählen.
            rate=rate+1;
        }
        s->sample_p[i]->class=class;
    }
    fprintf(stderr,"Ab jetzt nurnoch 'negative'\n");
    for(int i=0;i<s->count_n;i++){
        classold=s->sample_n[i]->class;
        pointSet(tmp,0.0);
        pointAdd(tmp,s->sample_n[i]);
        pointSub(tmp,a);
        yx=dotP(tmp,n);
        if(yx>=0){
            class=1;
        }else{
            class=0;
        }
        if(classold==class){
            rate=rate+1;
        }
        s->sample_n[i]->class=class;
    }
    printSamples(s);  //ausgeben der Klassifizierten Punkte
    fprintf(stderr,"Klassifizierung von %d Punkten abgeschlossen.\n",s->count_n+s->count_p);
    fprintf(stderr,"Trefferquote: %lf %%\n",rate/(s->count_n+s->count_p)*100);    
    destroyHuller(h);
    destroySamples(s);
    destroyPoint(n);
    destroyPoint(a);
    destroyPoint(tmp);
}

//Huller initialisieren
//Die Summe der alpha i (für positive) und alpha j (für negative) muss 1 sein
void initHuller(huller* h,samples* s){
   //Avg von Punkten berechnen
   int k=0;
   point *tmp = createPoint((s->sample_p[0])->dim);
   for(int i=0;i<AVGCOUNT;i++){ //AVGCOUNT viele positive Punkte suchen
        k=random()%(s->count_p);
        pointAdd(tmp,s->sample_p[k]); //Punkt aufaddieren
        s->sample_p[k]->alpha=((double)1)/((double)AVGCOUNT);  //alpha auf 1/avgcount setzen
    }   //--> in tmp liegt jetzt die summe der punkte
    pointDiv(tmp,AVGCOUNT);
    pointCopy(h->Xp,tmp);
    destroyPoint(tmp);
    tmp = createPoint((s->sample_n[0])->dim);
    for(int i=0;i<AVGCOUNT;i++){ //AVGCOUNT viele negative Punkte suchen
        k=random()%(s->count_n);
        pointAdd(tmp,s->sample_n[k]);
        s->sample_n[k]->alpha=((double)1)/((double)AVGCOUNT);  //alpha auf 1/avgcount setzen
    }     //--> in n liegt jetzt die summe der punkte
    pointDiv(tmp,AVGCOUNT);
    pointCopy(h->Xn,tmp);
    destroyPoint(tmp);
    //Xp und Xn stehen jetzt. Jetzt noch Skalare berechen.
    updateScalars(h);
    //und zuletzt die Alphalisten erstellen
    for(int i=0;i<s->count_p;i++){  //alle positiven durchiterieren
       addAlpha(s->p,i,s->sample_p[i]->alpha);  //punkt in alpha-liste p packen
    }
    for(int i=0;i<s->count_n;i++){
        addAlpha(s->n,i,s->sample_n[i]->alpha); //punkt in alpha-liste n packen
    }
}

//Huller updaten   index= index des punktes
void updateHuller(huller* h, samples* s,point* xn){
    double Xpxn = dotP(h->Xp,xn);
    double Xnxn = dotP(h->Xn,xn);
    double xnxn = dotP(xn,xn);
    double alpha_k = xn->alpha;
    double lambda_u=0.0;
    double lambda = 0.0;    
    double alpha_old = 0.0;
    //LAMBDA_U BERECHNEN
    if(xn->class==0){ //negativ Gleichung 5
        lambda_u = ((h->XnXn)-(h->XnXp)-Xnxn+Xpxn)/((h->XnXn)+xnxn-2*Xnxn);
    }else{  //positiv  Gleichung 4
        lambda_u = ((h->XpXp)-(h->XnXp)-Xpxn+Xnxn)/((h->XpXp)+xnxn-2*Xpxn);
    }
    lambda = min(1,max((-(xn->alpha))/(1-(xn->alpha)),lambda_u)); //lambda berechnen
    //ALLE ALPHAS DER GLEICHEN KLASSE UPDATEN
    if(xn->class==0){
        for(int i=0;i<s->count_n;i++){
            alpha_old = s->sample_n[i]->alpha;  //altes alpha speichern
            s->sample_n[i]->alpha=(1-lambda)*s->sample_n[i]->alpha;  //alpha_i = (1-l)*alpha_i    = Alpha updaten
            if(hasAlphaChanged(alpha_old,s->sample_n[i]->alpha)==1){   //hat sich das alpha verändert?
                  switchPoint(s->n,i,s->sample_n[i]->alpha); //Ja; verschiebe anhand des alphas den index in die andere liste
            }
        }
    }else{
       for(int i=0;i<s->count_p;i++){
            alpha_old = s->sample_p[i]->alpha;
            s->sample_p[i]->alpha=(1-lambda)*s->sample_p[i]->alpha;  //das selbe nur für positiv
            if(hasAlphaChanged(alpha_old,s->sample_p[i]->alpha)==1){
                switchPoint(s->p,i,s->sample_p[i]->alpha);  //alphaliste der postiven ändern/updaten
            }
        }
    }
    alpha_old = xn->alpha;
    xn->alpha = alpha_k+lambda;     //alpha vorm aktuellen punkt updaten
    if(hasAlphaChanged(alpha_old,xn->alpha)==1){   //falls sich alpha geändert hat
        if(xn->class==0){
            switchPoint(s->n,xn->index,xn->alpha); //alphaliste n updaten (weil xn negativ ist), index von xn ist in xn gespeichert
        }else{
            switchPoint(s->p,xn->index,xn->alpha);
        }
    }
    if(xn->class==1){ //Update XpXp XnXp XnXn -> Gleichungen 7
        h->XpXp = (1-lambda)*(1-lambda)*(h->XpXp)+2*lambda*(1-lambda)*Xpxn+lambda*lambda*xnxn;
        h->XnXp = (1-lambda)*(h->XnXp)+lambda*Xnxn;
      //h->XnXn = h->XnXn
    }else{ //Update XpXp XnXp XnXn -> Gleichungen 8
      //h->XpXp = h->XpXp
        h->XnXp = (1-lambda)*(h->XnXp)+lambda*Xpxn;
        h->XnXn = (1-lambda)*(1-lambda)*(h->XnXn)+2*lambda*(1-lambda)*Xnxn+lambda*lambda*xnxn;
    }
}



//Haupte Hullerschleife
void mainHuller(huller* h, samples* s){
    initHuller(h,s);    //Huller initialisieren
    int dauer=0;
    for(int i=0;i<MAXITERATIONS;i++){
        if(alphaNotNull(s) < CONVERGE)
        break; //konvergiert, also beenden.
        if(i%(MAXITERATIONS/20)==0)
        fprintf(stderr,"Lerne... %lf%% (%d/%d) \n",(((double)i)/((double)MAXITERATIONS))*100,i,MAXITERATIONS);
        
        point* p=randPoint(s,0); //zufälligen punkt mit alpha=0
        updateHuller(h,s,p);

        point* r=randPoint(s,1); //zufälligen punkt mit alpha!=0
        updateHuller(h,s,r); //Update auf Punkt starten
        dauer=i;
    }
    fprintf(stderr,"Fertig nach %d Generationen\n",dauer);
    //jetzt noch XP und XN ändern.
    point *tmp=createPoint(h->Xp->dim);
    point *tmp2=createPoint(h->Xp->dim);
    for(int i=0;i<s->count_p;i++){  //Xp berechnen (Xp = sum_i(x_i*alpha_i)
        pointSet(tmp2,0);       //tmp2 leer
        pointAdd(tmp2,s->sample_p[i]);  //tmp2 = x
        pointMult(tmp2,s->sample_p[i]->alpha);    //tmp2 = x*alpha    
        pointAdd(tmp,tmp2);         //tmp += tmp2 (x*alpha)
    }
    pointCopy(h->Xp,tmp);
    pointSet(tmp,0);
    for(int i=0;i<s->count_n;i++){  //Xn berechnen
        pointSet(tmp2,0),
        pointAdd(tmp2,s->sample_n[i]);
        pointMult(tmp2,s->sample_n[i]->alpha);
        pointAdd(tmp,tmp2);
    }
    pointCopy(h->Xn,tmp);
    //funktion ausgeben
    //fprintf(stderr,"\ny(x)=(");
    //prettyPrint(h->Xp);
    //fprintf(stderr,"-");
    //prettyPrint(h->Xn);
    //fprintf(stderr,"x+%lf-%lf)/2\n",h->XnXn,h->XpXp);
    destroyPoint(tmp);
    destroyPoint(tmp2);
    alphaStats(s);

}
















/*
Ab hier nurnoch Testfunktionen um diverse Funktionen und Datenstrukturen zu testen
*/

//datei einlesen -> keine speicherfehler
void testFile(char *input,int dim){
    printf("\n\nTeste Dateieinlesen\n\n");
    //erstmal nur testweise eine leere samplemenge erzeugen
    samples *s = createSamples();
    readSamples(input,dim,s);
    printSamples(s);
    destroySamples(s);
    getchar();
}

void testHINIT(){
    printf("\n\nTeste hullerInit\n\n");
    samples *s = createSamples();
    readSamples("a1a.svm",123,s);
    huller *h = createHuller(123);
    printf("Vor init alphaStats\n");
    alphaStats(s);
    initHuller(h,s);
    printf("Nach init AlphaStats\n");
    alphaStats(s);
    destroyHuller(h);
    destroySamples(s);

}

//Test der Point-Datenstruktur: konstruktur, destruktor, Skalarprodukt und avg-Operation
//--> Keine Speicherfehler
void testPoint(){
    printf("\n\nTeste Punkte\n\n");
	point *p1;
    p1 = createPoint(3);
    p1->coords[0]=2;
    p1->coords[1]=4;
    p1->coords[2]=6;
    point *p2;
    p2 = createPoint(3);
    p2->coords[0]=5;
    p2->coords[1]=10;
    p2->coords[2]=15;
    printPoint(p1);
    printPoint(p2);
    avgPoints(p1,p2);
    printf("Avg der beiden:\n");
    printPoint(p1);
    destroyPoint(p1);
    destroyPoint(p2);
    getchar();
}

//Komponente zu nem Punkt hinzufügen -->keine speicherfehler
void testAddComp(){
       printf("\n\nTest - Komponenten hinzufügen\n\n");
       point *p=createPoint(2);
       p->coords[0]=1;
       p->coords[1]=2;
       printPoint(p);
       for(int i=0;i<100;i++){
           pointAddComp(p,i);
       }
       printPoint(p);
       destroyPoint(p);
       getchar();
}
//init von Huller testen./bu    
void testInit(){
    printf("\n\nTest - Huller initialisieren\n");
    huller *h=createHuller(2);
    samples *s=createSamples();
    for(int i=0;i<100;i++){
        sampleAdd(s,randomPoint(2));
    }
    printSamples(s);
    printf("|P|=%d |N|=%d\n",s->count_p,s->count_n);
    initHuller(h,s);
    printf("Startwert Xp:\n");
    printPoint(h->Xp);
    printf("Startwert Xn:\n");
    printPoint(h->Xn);
    destroySamples(s);
    printf("XpXp: %lf XnXp: %lf XnXn: %lf\n",h->XpXp,h->XnXp,h->XnXn);
    destroyHuller(h);
}


//Sample erstellen, füllen und löschen --> keine speicherfehler
void sampleTest(){
    printf("\n\nTest - Samples hinzufügen\n\n");
    samples *s=createSamples();
    for(int i=0;i<100;i++){
        sampleAdd(s,randomPoint(3));
    }
    printSamples(s);
    getchar();
    printf("\n\nJetzt ein paar zufällige Punkte aus Samples:\n\n");
    for(int i=0;i<10;i++){
        printPoint(randPoint(s,random()%2));
    }    
    destroySamples(s);
    getchar();
}

//Eine richtige svm Datei einlesen und Dinge damit anstellen
//Erkennt alle 1605 Punkte. 395 Positiv, 1210 negativ //zeit < 1ns
void completeTest(){
    printf("\n\n 'Richtiger' test\n\n");
    //debug ausschalten
    //debug=0;
    //struct timespec time_before, time_after;
    samples *s=createSamples();
    //clock_gettime(CLOCK_MONOTONIC, &time_before);
    readSamples("a1a.svm",123,s);
    //clock_gettime(CLOCK_MONOTONIC, &time_after);
    printSamples(s);
    destroySamples(s);
   // printf("Dauer für das Einlesen von 1605 Samples: %lf\n",(time_after.tv_sec - time_before.tv_sec)*(1000000000l) + (time_after.tv_nsec - time_before.tv_nsec));

}

void loadHullerTest(){
    printf("Test, ob Huller geladen werden kann\n\n");
    huller *h=hullerFromFile("modell");
    printHuller(h);
    destroyHuller(h);

}


double min(double a, double b){
    if(a<=b){
        return a;
    }else{
    return b;
    }
}

double max(double a, double b){
    if(a>=b){
        return a;
    }else{
        return b;
    }
}

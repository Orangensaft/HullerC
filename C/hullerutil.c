#include <stdio.h>
#include "point.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#define debug 0

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

//Konstruktor für Huller
huller *createHuller(int dim){
    huller *h = malloc(sizeof(huller));
    h->Xp = createPoint(dim);
    (h->Xp)->class=1;
    h->Xn = createPoint(dim);
    (h->Xn)->class=0;
    h->XpXp = 0.0;
    h->XnXp = 0.0;
    h->XnXn = 0.0;
    return h;
}

huller *hullerFromFile(char* file){
    int dim=0;
    int state=0; //0->Dimension lesen 1> xn lesen 2->xp lesen 3->pp 4->np->5nn
    FILE *hullerfile = fopen(file,"r"); //hullerdatei öffnen
    char *buf = malloc(10000);
    int i=0;
    huller *h;
    while(fgets(buf,10000,hullerfile)){ //in buf liegt jetzt immer die aktuelle zeile.
        if(state==0){
            if(strncmp(buf,"xn",2)==0){
                state=1;
                continue;
            }else{
                dim=atoi(buf);
                h = createHuller(dim);
            }
        }
        if(state==1){
            if(strncmp(buf,"xp",2)==0){
                state=2;
                i=0;
                continue;
            }else{
                h->Xn->coords[i]=strtof(buf,NULL);
                i=i+1;
            }
        }
        if(state==2){
            if(strncmp(buf,"xpxp",4)==0){
                state=3;
                i=0;
                continue;
            }else{
                h->Xp->coords[i]=strtof(buf,NULL);
                i=i+1;
            }
        }
        if(state==3){
            if(strncmp(buf,"xnxp",4)==0){
                state=4;
                continue;
            }else{
                h->XpXp=strtof(buf,NULL);
            }
        }
        if(state==4){
            if(strncmp(buf,"xnxn",4)==0){
                state=5;
                continue;
            }else{
                h->XnXp=strtof(buf,NULL);
            }
        }
        if(state==5){
            if(strncmp(buf,"ende",4)==0){
                break;
            }else{
                h->XnXn=strtof(buf,NULL);
            }
        }
    }
    fclose(hullerfile); //datei auch wieder schließen
    free(buf);
    return h;
}

//Aus einem Sample einen zufälligen Punkt ziehen (kann positiv oder negativ sein)
//Gibt pointer auf punkt zurück.
point* randPoint(samples* s){
    int k=0;    
    k=random()%2; //positiver oder negativer punkt?
    if(k==0){
     k=random()%(s->count_n);
     return (s->sample_n[k]);   
    }
    else{
      k=random()%(s->count_p);
      return (s->sample_p[k]);
    }
}

void printHuller(huller *h){
    printf("%d\n",h->Xn->dim); //anzahl dimensionen
    printf("xn\n");
    for(int i=0;i<h->Xn->dim;i++){
        printf("%lf\n",(h->Xn->coords[i]));
    }
    printf("xp\n");
    for(int i=0;i<h->Xp->dim;i++){
        printf("%lf\n",(h->Xp->coords[i]));
    }
    printf("xpxp\n");
    printf("%lf\n",h->XpXp);
    printf("xnxp\n");
    printf("%lf\n",h->XnXp);
    printf("xnxn\n");
    printf("%lf\n",h->XnXn);
    printf("ende");

}
//Skalare von Huller updaten (nur im init genutzt)
void updateScalars(huller *h){
h->XpXp=dotP(h->Xp,h->Xp);
h->XnXp=dotP(h->Xn,h->Xp);
h->XnXn=dotP(h->Xn,h->Xn);
}

//Destruktor
void destroyHuller(huller *h){
    destroyPoint(h->Xp);
    destroyPoint(h->Xn);
    free(h);
}

//Punkt zu sample hinzufügen
void sampleAdd(samples* s,point* p){
    if(p->class==0){ //Negativ
        s->count_n=s->count_n+1;
        s->sample_n=realloc(s->sample_n, (s->count_n)*sizeof(point*));  //
        s->sample_n[(s->count_n)-1]=p; 
    }else{  //positiv
        s->count_p=s->count_p+1;
        s->sample_p=realloc(s->sample_p, (s->count_p)*sizeof(point*));
        s->sample_p[(s->count_p)-1]=p;    
    }
}


/*
//Samples einlesen -> dim dimensionen maximal.
//gehe von folgender formatierung aus:
(+|-)1 1:%lf 2:%lf ... dim:%lf
Wobei Attribute die nicht auftauchenden default 0 gesetzt werden
Bsp:
-1 3:1 6:1 17:1 27:1 35:1 ...
*/
//fscanf kann hier nicht verwendet werden weil Zeilen verschiedene viele Einträge haben
void readSamples(char *file,int dim,samples *s){
    FILE *svmfile = fopen(file,"r");    
    int len=0;    
    int wlen=0;
    int matches=0;
    int lasthit=0;
    int dimIndex=0;
    double value=0.0;
    point *p;
    char *curComp = malloc(500000); //aktuelle komponente
    char *buf = malloc(500000); //wir wissen nicht wie lang eine zeile maximal werden kann
    while(fgets(buf,500000,svmfile)){ //zeilenweises lesen
        //*buf ist die aktuelle Zeile, somit unser aktueller Punkt
        p=createPoint(dim); //neuen Punkt, Dimension ist ja gegeben.
        matches=0; //wir haben noch keine komponenten gefunden
        lasthit=0;
        len=strlen(buf);
        for(int i=0;i<len;i++){
            if(buf[i]==' '){  //mit /n
                matches+=1;
                if(debug){
                    printf("Leerzeichen bei %d gefunden. Letzter Treffer war bei %d\nDas war Treffer %d\n",i,lasthit,matches);
                }
                if(matches==1){ //erste komponente gefunden, also die Klassifizierung
                    if(buf[0]=='+'){ //positives Beispiel
                        p->class=1;
                    }
                    if(buf[0]=='-'){ //negatives Beispiel
                        p->class=0;
                    }
                }else{ //ab jetzt kommen punkte
                    wlen=(i - lasthit)-1;
                    if(debug)
                        printf("Länge: %d\n",wlen);
                    strncpy(curComp,buf+lasthit+1,wlen);
                    curComp[wlen]='\0';
                    if(debug)
                        printf("Komponente: %s\n",curComp);
                    sscanf(curComp,"%d:%lf",&dimIndex,&value);
                    if(debug)                    
                        printf("Dim :%d - Value:%lf\n",dimIndex,value);
                    assert(dimIndex<p->dim);
                    p->coords[dimIndex]=value;
                }
                lasthit=i;
            }if(buf[i]=='\n'){      //Abfrage für \n am ende.
                if(lasthit==i-1){
                    //nix tun, direkt davor kam ein lerzeichen, zeile ist also zu ende.
                }else{
                    printf("\\n gefunden ohne leerzeichen davor!\n");
                    wlen=(i - lasthit)-1;
                    if(debug)
                        printf("Länge: %d\n",wlen);
                    strncpy(curComp,buf+lasthit+1,wlen);
                    curComp[wlen]='\0';
                    if(debug)
                        printf("Komponente: %s\n",curComp);
                    sscanf(curComp,"%d:%lf",&dimIndex,&value);
                    if(debug)                    
                        printf("Dim :%d - Value:%lf\n",dimIndex,value);
                    assert(dimIndex<p->dim);
                    p->coords[dimIndex]=value; 
                }

            }
        }
        //hier punkt zu sample hinzufügen
        sampleAdd(s,p);
        if(debug)
            printf("%d Komponenten gefunden\n",matches);
        
    }
    free(buf);
    free(curComp);
    fclose(svmfile);
}


//samples erstelen (leer)
samples* createSamples(){
    samples *s=malloc(sizeof(samples));
    s->count_n=0;
    s->count_p=0;
    s->sample_n=malloc(sizeof(point*));
    s->sample_p=malloc(sizeof(point*));
    return s;
}

//destruktor von sample
void destroySamples(samples* s){
    for(int i=0;i<s->count_n;i++){
        destroyPoint(s->sample_n[i]); //einzelnen punkte löschen
    }  
    for(int i=0;i<s->count_p;i++){
        destroyPoint(s->sample_p[i]);
    }
    free(s->sample_n);            //array von zeigern löschen
    free(s->sample_p);    
    free(s);                //s selbst löschen

}

//sample ausgeben
void printSamples(samples* s){
    for(int i=0;i<s->count_p;i++){
         printPoint(s->sample_p[i]);
    }
    for(int i=0;i<s->count_n;i++){
         printPoint(s->sample_n[i]);
    }
}


int alphaNotNull(samples *s){
    int sumnotnull=0;
    for(int i=0; i<s->count_n;i++){
        if(s->sample_n[i]->alpha!=0)
            sumnotnull=sumnotnull+1;
    }
    for(int i=0; i<s->count_p;i++){
        if(s->sample_p[i]->alpha!=0)
            sumnotnull=sumnotnull+1;
    }
    return sumnotnull;
}

void alphaStats(samples *s){
    fprintf(stderr,"\n-#-#-#-#-#-#- Alpha-Statistik -#-#-#-#-#-#-\n");
    int sumnull=0;
    int sumnotnull=0;
    for(int i=0;i<s->count_n;i++){
        if(s->sample_n[i]->alpha==0){
            sumnull=sumnull+1;
        }else{
            sumnotnull=sumnotnull+1;
        }
    }
    for(int i=0;i<s->count_p;i++){
        if(s->sample_p[i]->alpha==0){
            sumnull=sumnull+1;
        }else{
            sumnotnull=sumnotnull+1;        
        }
    }
    fprintf(stderr,"Anzahl alpha=0 : %d\n",sumnull);
    fprintf(stderr,"Anzahl alpha!=0: %d\n",sumnotnull);
}



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
//Konstanten
//Anzahl der Punkte die für die Initialisierung genutzt werden
#define AVGCOUNT 500
//Maximale Anzahl der Iterationen
#define MAXITERATIONS 100000
// je größer desto genauer
#define CONVERGE 100
//anzahl alpha!=0 zum abbrechen

int debug=0;

typedef struct{
	double* coords;
	int dim;
    int class; //0 -> -; 1 -> +;
    double alpha;
} point;

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

point *createPoint(int dim);
void destroyPoint(point *p);
double dotP(point *p1,point *p2);
point *randomPoint(int dim);
void printPoint(point *p);
void avgPoints(point *p1, point *p2);
void testPoint();
void testFile(char *input,int dim);
void testAddComp();
void sampleTest();
void pointAddComp(point *p,double val);
samples* createSamples();
void sampleAdd(samples* s,point* p);
void readSamples(char *file,int dim,samples *s);
void destroySamples(samples* s);
void initHuller(huller* h,samples* s);
void testInit();
void pointCopy(point* dest, point* src);
void pointAdd(point *p1, point *p2);
void pointSub(point *p1, point *p2);
void pointMult(point *p1,double n);
void pointSet(point *p1,double n);
void updateScalars(huller *h);
void destroyHuller(huller *h);
huller *createHuller(int dim);
huller *hullerFromFile(char* file);
void mainHuller(huller* h, samples* s);
void updateHuller(huller* h, samples* s,point* xn);
point* randPoint(samples* s);
void completeTest();
double max(double a, double b);
double min(double a, double b);
void prettyPrint(point* p);
void printSamples(samples* s);
void classify(char* svmfile, char* hulfile,int dim);
void learn(char* svmfile,int dim);
void printHuller(huller *h);
void testHINIT();
void alphaStats(samples *s);
void loadHullerTest();
int alphaNotNull(samples *s);

int main(int argc, char **argv){
    srandom((int)time(NULL));
    if(argc==1){
        printf("Keine Argumente angegeben; Wechsele in Testmodus\n");
        getchar();
        loadHullerTest();
        //testHINIT();
        //debug=1;
        //testPoint();
        //sampleTest();    
        //testFile("testinput.svm",150);
       // testAddComp();   
        //testInit();
        //completeTest();        
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

//punkt im dim-dimensionalen raum erstellen
point *createPoint(int dim){
    assert(dim>=0);
	point *p=malloc(sizeof(point));	//speicher für struct reservieren
	p->dim=dim;
	p->coords=calloc(dim,sizeof(double)); //speicher für double-array (dim*sizeof(double))
    p->class=0;
    p->alpha=0;
	return p;
}

/*dem punkt eine Komponente hinzufügen
dadurch wäre es theoretisch auch möglich einzulesen ohne die Dimension zu kennen.
->Starte bei Dimension 0. Wenn ein neues Attribut auftaucht (zB. 15:1) die Dimension darauf erhöhen.
->Das allerdings auch rückwirkend auf alle anderen Punkte. -> Extreme Steigerung der Laufzeit
*/
void pointAddComp(point *p,double val){
    p->coords = realloc(p->coords, sizeof(double)*(p->dim+1));
    p->dim=p->dim+1;
    p->coords[p->dim-1]=val; //mit 0 initialisieren
}

//punkt freigeben
void destroyPoint(point *p){
	free(p->coords);
	free(p);
}

//skalarprodukt
double dotP(point *p1, point *p2){
	assert(p1->dim == p2->dim);
	double ret=0.0;   //zwangsweise initialisieren?	
	for(int i=0;i<p1->dim;i++){
		ret+=(p1->coords[i])*(p2->coords[i]);	
	}	
	return ret;
}

//zufälligen punkt erstellen zum testen
point *randomPoint(int dim){
	point *p=createPoint(dim);
	for(int i=0;i<dim;i++){
		p->coords[i]=(double)(random()/random());
	}
    p->class=random()%2;
	return p;
}


//punkt ausgeben zum testen und zum speichern der samples.
void printPoint(point *p){
	//printf("Dimension: %d Klassifizierung: %d\n",(int)p->dim,p->class);
	//printf("<");
    if(p->class==0)
        printf("-1 ");
    if(p->class==1)
        printf("+1 ");
	for(int i=0; i < p->dim; i++){
		if(p->coords[i]!=0){
			printf("%d:%lf ",i,p->coords[i]);
							
		}
	}
	printf("\n");
}

//durchschnitt zwischen 2 punkten errechnen
void avgPoints(point *p1, point *p2){
	assert(p1->dim==p2->dim);	
	for(int i=0;i<p1->dim;i++){
		p1->coords[i]=((p1->coords[i])+(p2->coords[i]))/2;
	}
}


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

//punkt kopieren -> komponentenweise
void pointCopy(point* dest, point* src){
    assert(dest->dim == src->dim);
    for(int i=0;i<dest->dim;i++){
        dest->coords[i]=src->coords[i];
    }
}

//p1 = p1+p2;
void pointAdd(point *p1, point *p2){
    assert(p1->dim==p2->dim);
    for(int i=0;i<p1->dim;i++){
        p1->coords[i] += p2->coords[i];
    }
}

//p1=p1-p2;
void pointSub(point *p1, point *p2){
    assert(p1->dim==p2->dim);
    for(int i=0;i<p1->dim;i++){
        p1->coords[i] -= p2->coords[i];
    }
}

//Punkt durch n Teilen
void pointDiv(point *p1,double n){
    for(int i=0;i<p1->dim;i++){
           p1->coords[i]=(p1->coords[i])/n;
     }
}

void pointMult(point *p1,double n){
    for(int i=0;i<p1->dim;i++){
        p1->coords[i]=(p1->coords[i])*n;
    }
}
//alle komponenten auf n setzen
void pointSet(point *p1,double n){
    for(int i=0;i<p1->dim;i++){
        p1->coords[i]=n;
    }
}

//Huller initialisieren
//Die Summe der alpha i (für positive) und alpha j (für negative) muss 1 sein
//-> alpha_i = 1/count_p
//-> alpha_j = 1/count_n
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



//Huller updaten
void updateHuller(huller* h, samples* s,point* xn){
    double Xpxn = dotP(h->Xp,xn);
    double Xnxn = dotP(h->Xn,xn);
    double xnxn = dotP(xn,xn);
    double alpha_k = xn->alpha;
    double lambda_u=0.0;
    double lambda = 0.0;    
    if(xn->class==0){ //negativ Gleichung 5
        lambda_u = ((h->XnXn)-(h->XnXp)-Xnxn+Xpxn)/((h->XnXn)+xnxn-2*Xnxn);
    }else{  //positiv  Gleichung 4
        lambda_u = ((h->XpXp)-(h->XnXp)-Xpxn+Xnxn)/((h->XpXp)+xnxn-2*Xpxn);
    }
    lambda = min(1,max((-(xn->alpha))/(1-(xn->alpha)),lambda_u)); //lambda berechnen
    if(xn->class==0){
        for(int i=0;i<s->count_n;i++){
            s->sample_n[i]->alpha=(1-lambda)*s->sample_n[i]->alpha;  //alpha_i = (1-l)*alpha_i
        }
    }else{
       for(int i=0;i<s->count_p;i++){
            s->sample_p[i]->alpha=(1-lambda)*s->sample_p[i]->alpha;  //das selbe nur für positiv
        }
    }
    xn->alpha = alpha_k+lambda;     //alpha vorm aktuellen punkt updaten
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

//Haupte Hullerschleife
void mainHuller(huller* h, samples* s){
    initHuller(h,s);    //Huller initialisieren
    for(int i=0;i<MAXITERATIONS;i++){
        if(alphaNotNull(s) < CONVERGE)
        break; //konvergiert, also beenden.
        if(i%(MAXITERATIONS/20)==0)
        fprintf(stderr,"Lerne... %lf%% (%d/%d) \n",(((double)i)/((double)MAXITERATIONS))*100,i,MAXITERATIONS);
        point* p=randPoint(s);
        //alphaStats(s);
        while(p->alpha!=0){  //Solange random bis wir einen punkt mit alpha=0 finden
            p=randPoint(s);
        }
        updateHuller(h,s,p); //Update auf Punkt starten
        point* r=randPoint(s);
        //alphaStats(s);
        while(r->alpha==0){  //Zufälligen punkt mit alpha != 0 suchen
            r=randPoint(s);
        }
        updateHuller(h,s,r); //Update auf Punkt starten
    }
    //jetzt noch XP und XN ändern.
    point *tmp=createPoint(h->Xp->dim);
    point *tmp2=createPoint(h->Xp->dim);
    for(int i=0;i<s->count_p;i++){  //Xp berechnen
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
    //alphaStats(s);

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

void prettyPrint(point* p){
    for(int i=0;i<(p->dim);i++){
        fprintf(stderr,"%lf",(p->coords[i]));
        if(i!=(p->dim-1))
            fprintf(stderr,",");
    }
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
        printPoint(randPoint(s));
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

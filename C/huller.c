#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
//Konstanten
//Anzahl der Punkte die für die Initialisierung genutzt werden
#define AVGCOUNT 10

int debug=0;

typedef struct{
	float* coords;
	int dim;
    int class; //0 -> -; 1 -> +;
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
   float XpXp;
   float XnXp;
   float XnXn;
} huller;

point *createPoint(int dim);
void destroyPoint(point *p);
float dotP(point *p1,point *p2);
point *randomPoint(int dim);
void printPoint(point *p);
void avgPoints(point *p1, point *p2);
void testPoint();
void testFile(char *input,int dim);
void testAddComp();
void sampleTest();
void pointAddComp(point *p,float val);
samples* createSamples();
void sampleAdd(samples* s,point* p);
void destroySamples(samples* s);
void initHuller(huller* h,samples* s);

int main(int argc, char **argv){
    srandom((int)time(NULL));
    if(argc==1){
        printf("Keine Argumente angegeben; Wechsele in Testmodus\n");
        getchar();
        debug=1;
        testPoint();
        sampleTest();    
        testFile("testinput.svm",150);
        testAddComp();   
        exit(EXIT_SUCCESS);
    }
}

//punkt im dim-dimensionalen raum erstellen
point *createPoint(int dim){
    assert(dim>=0);
	point *p=malloc(sizeof(point));	//speicher für struct reservieren
	p->dim=dim;
	p->coords=calloc(dim,sizeof(float)); //speicher für float-array (dim*sizeof(float))
    p->class=0;
	return p;
}

/*dem punkt eine Komponente hinzufügen
dadurch wäre es theoretisch auch möglich einzulesen ohne die Dimension zu kennen.
->Starte bei Dimension 0. Wenn ein neues Attribut auftaucht (zB. 15:1) die Dimension darauf erhöhen.
->Das allerdings auch rückwirkend auf alle anderen Punkte. -> Extreme Steigerung der Laufzeit
*/
void pointAddComp(point *p,float val){
    p->coords = realloc(p->coords, sizeof(float)*(p->dim+1));
    p->dim=p->dim+1;
    p->coords[p->dim-1]=val; //mit 0 initialisieren
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
		p->coords[i]=(float)(random()/random());
	}
	return p;
}


//punkt ausgeben zum testen
void printPoint(point *p){
	printf("Dimension: %d Klassifizierung: %d\n",(int)p->dim,p->class);
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
    h->Xn = createPoint(dim);
    h->XpXp = 0.0;
    h->XnXp = 0.0;
    h->XnXn = 0.0;
    return h;
}

void initHuller(huller* h,samples* s){
   //TODO: avg von ein paar positiven und negativen punkten bilden
   //TODO: Xp, Xn, XpXp, XnXp, XnXn berechnen
    int k=0;
    int j=0;
   for(int i=0;i<AVGCOUNT;i++){
        k=random()%(s->count_p);
        j=random()%(s->count_p);        
    }

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
(+|-)1 1:%f 2:%f ... dim:%f
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
    float value=0.0;
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
            if(buf[i]==' '){
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
                    sscanf(curComp,"%d:%f",&dimIndex,&value);
                    if(debug)                    
                        printf("Dim :%d - Value:%f\n",dimIndex,value);
                    p->coords[dimIndex]=value;
                }
                lasthit=i;
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

void sampleAdd(samples* s,point* p){
    if(p->class==0){ //Negativ
        s->count_n=s->count_n+1;
        s->sample_n=realloc(s->sample_n, (s->count_n)*sizeof(point));  //nicht größe von *point?  
        s->sample_n[(s->count_n)-1]=p; 
    }else{  //positiv
        s->count_p=s->count_p+1;
        s->sample_p=realloc(s->sample_p, (s->count_p)*sizeof(point));
        s->sample_p[(s->count_p)-1]=p;    
    }
}

//samples erstelen (leer)
samples* createSamples(){
    samples *s=malloc(sizeof(samples));
    s->count_n=0;
    s->count_p=0;
    s->sample_n=malloc(sizeof(point));
    s->sample_p=malloc(sizeof(point));
    return s;
}

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

void printSamples(samples* s){
    printf("Positive Beispiele:\n");
    for(int i=0;i<s->count_p;i++){
         printPoint(s->sample_p[i]);
    }
    printf("Negative Beispiele:\n");
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
    //TODO: Mit den Samples weiterarbeiten
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


//Sample erstellen, füllen und löschen --> keine speicherfehler
void sampleTest(){
    printf("\n\nTest - Samples hinzufügen\n\n");
    point *p1=createPoint(3);
    p1->coords[0]=5;
    p1->coords[1]=6;
    p1->coords[2]=2;    
    point *p2=createPoint(3);
    p2->coords[0]=1;
    p2->coords[1]=3;
    p2->coords[2]=3;
    p2->class=42;
    samples *s=createSamples();
    sampleAdd(s,p1);
    sampleAdd(s,p2);
    printSamples(s);
    destroySamples(s);
    getchar();
}

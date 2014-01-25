#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define debug 1

typedef struct{
	float* coords;
	int dim;
    int class; //0 -> -; 1 -> +;
} point;

//Array aus count vielen Punkten
typedef struct{
    int count;
    point *sample;
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
point *avgPoints(point *p1, point *p2);
void testPoint();
void testFile(char *input,int dim);
void testAddComp();
void pointAddComp(point *p,float val);


int main(int argc, char **argv){
    srandom((int)time(NULL));
    testPoint();
    //testFile("testinput.svm",150);
    //testAddComp();
}

//punkt im dim-dimensionalen raum erstellen
point *createPoint(int dim){
    assert(dim>=0);
	point *p=malloc(sizeof(point));	//speicher für struct reservieren
	p->dim=dim;
	p->coords=calloc(dim,sizeof(float)); //speicher für float-array (dim*sizeof(float))
    p->class=-1;
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

/*
//Samples einlesen -> dim dimensionen maximal.
//gehe von folgender formatierung aus:
(+|-)1 1:%f 2:%f ... dim:%f
Wobei Attribute die nicht auftauchenden default 0 gesetzt werden
Bsp:
-1 3:1 6:1 17:1 27:1 35:1 ...
*/
/*
Formel für die restlichen Komponenten (noch ungetestet, sollte aber hinhauen. Auch für erste komponente wenn lasthit=-1 zu beginn
Wortlänge = (neues_leerzeichen - letztes)-1
Wort = (buf+(letztes+1))
dementsprechend dann:
strncpy(wort,buf+lasthit+1,(i-lasthit)-1)
*/
//fscanf kann hier nicht verwendet werden weil Zeilen verschiedene viele Einträge haben
void readSamples(char *file,int dim,samples *s){
    FILE *svmfile = fopen(file,"r");    
    int len=0;    
    int compLen=0;
    int matches=0;
    int lasthit=0;
    point *p;
    char *curComp = malloc(500000); //aktuelle komponente
    char *buf = malloc(500000); //wir wissen nicht wie lang eine zeile maximal werden kann
    while(fgets(buf,500000,svmfile)){ //zeilenweises lesen
        //*buf ist die aktuelle Zeile, somit unser aktueller Punkt
        p=createPoint(dim); //neuen Punkt, Dimension ist ja gegeben.
        matches=0; //wir haben noch keine komponenten gefunden
        lasthit=0;
        len=strlen(buf);
            if(debug)
               printf("Länge: %d\n",len);
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
                    //TODO: Die einzelnen Komponenten aus dem String ziehen. Sollte nicht schwer sein, in Ruhe nachdenken!
                }
                lasthit=i;
            }
        }
        if(debug)
            printf("%d Komponenten gefunden\n",matches);
        
    }
    free(buf);
    free(curComp);
    fclose(svmfile);
}


/*

Ab hier nurnoch Testfunktionen um diverse Funktionen und Datenstrukturen zu testen

*/

void testFile(char *input,int dim){
    //erstmal nur testweise eine leere samplemenge erzeugen
    samples *s = malloc(sizeof(samples));
    s->count=0;
    s->sample=NULL;
    readSamples(input,dim,s);
    //TODO: Mit den Samples weiterarbeiten
    free(s);
}


//Test der Point-Datenstruktur: konstruktur, destruktor, Skalarprodukt und avg-Operation
void testPoint(){
	point *p1;
    p1 = createPoint(3);
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
    p1 = createPoint(0);
    printPoint(p1);
    pointAddComp(p1,0.5);
    pointAddComp(p1,0.8);
    pointAddComp(p1,0.4);
    printPoint(p1);
    destroyPoint(p1);
}

void testAddComp(){
       printf("Test - Komponenten hinzufügen\n");
       point *p=createPoint(2);
       p->coords[0]=1;
       p->coords[1]=2;
       printPoint(p);
       for(int i=0;i<100;i++){
           pointAddComp(p,i);
       }
       printPoint(p);
       destroyPoint(p);
}


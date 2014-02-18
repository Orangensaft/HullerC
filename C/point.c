#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define debug 0

typedef struct{
	double* coords;
	int dim;
    int class; //0 -> -; 1 -> +;
    double alpha;
} point;

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

void prettyPrint(point* p){
    for(int i=0;i<(p->dim);i++){
        fprintf(stderr,"%lf",(p->coords[i]));
        if(i!=(p->dim-1))
            fprintf(stderr,",");
    }
}


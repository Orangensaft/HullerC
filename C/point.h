typedef struct{
    double* coords;
    int dim;
    int class;
    double alpha;
} point;

point *createPoint(int dim);
void destroyPoint(point *p);
double dotP(point *p1,point *p2);
point *randomPoint(int dim);
void printPoint(point *p);
void avgPoints(point *p1, point *p2);
void pointAddComp(point *p,double val);
void pointCopy(point* dest, point* src);
void pointAdd(point *p1, point *p2);
void pointSub(point *p1, point *p2);
void pointMult(point *p1,double n);
void pointSet(point *p1,double n);
void pointDiv(point *p1,double n);
void prettyPrint(point* p);

void calculateMview(float(&camera)[3], float(&cRotation)[3], float(&refPoint)[3], float(&mView)[4][4]);
void transform(float(&x)[10], float(&y)[10], float(&z)[10], float(&mView)[4][4], float(&finnalCoordinate)[10][3], float d, float f, float h);
void backCulling(float x[10], float y[10], float z[10], int(&isVisable)[10], float camera[3], int planenum, int plane[7][7], float mView[4][4]);
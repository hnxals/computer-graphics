#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
using namespace std;

const int B = 256;
const int BM = 255;
const int N = 4096;
const int NM = 4095;
const int NP = 12;
static int p[B * 2 + 2];
static float g2[B * 2 + 2][2];
bool start = true;
static void init(void);

typedef struct                       /**** BMP file header structure ****/
{
	unsigned int   bfSize;           /* Size of file */
	unsigned short bfReserved1;      /* Reserved */
	unsigned short bfReserved2;      /* ... */
	unsigned int   bfOffBits;        /* Offset to bitmap data */
} MyBITMAPFILEHEADER;


typedef struct                       /**** BMP file info structure ****/
{
	unsigned int   biSize;           /* Size of info header */
	int            biWidth;          /* Width of image */
	int            biHeight;         /* Height of image */
	unsigned short biPlanes;         /* Number of color planes */
	unsigned short biBitCount;       /* Number of bits per pixel */
	unsigned int   biCompression;    /* Type of compression to use */
	unsigned int   biSizeImage;      /* Size of image data */
	int            biXPelsPerMeter;  /* X pixels per meter */
	int            biYPelsPerMeter;  /* Y pixels per meter */
	unsigned int   biClrUsed;        /* Number of colors used */
	unsigned int   biClrImportant;   /* Number of important colors */
} MyBITMAPINFOHEADER;




float curve(float t) {
	float temp = 0;
	temp += 6 * pow(t, 5);
	temp -= 15 * pow(t, 4);
	temp += 10 * pow(t, 3);
	return temp;
}
float lerp(float t, float a, float b) {
	return a + t * (b - a);
}

float at2(float *q, float rx, float ry) {
	return rx * q[0] + ry * q[1];
}


void SetUp(float &t, float vec[2], int i, int &b0, int &b1, float &r0, float &r1) {
	t = vec[i] + N;
	b0 = ((int)t) & BM;
	b1 = (b0 + 1) & BM;
	r0 = t - (int)t;
	r1 = r0 - 1;
}

float PerlinNoise(float vec[2])
{
	srand(time(nullptr));
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, * q, sx, sy, a, b, u, v, t;
	register int i, j;
	if (start) {
		start = false;
		init();
	}
	SetUp(t, vec, 0, bx0, bx1, rx0, rx1);
	SetUp(t, vec, 1, by0, by1, ry0, ry1);
	i = p[bx0];
	j = p[bx1];
	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];
	sx = curve(rx0);
	sy = curve(ry0);
	q = g2[b00]; u = at2(q, rx0, ry0);
	q = g2[b10]; v = at2(q, rx1, ry0);
	a = lerp(sx, u, v);
	q = g2[b01]; u = at2(q, rx0, ry1);
	q = g2[b11]; v = at2(q, rx1, ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);

}
static void normalize(float v[2])
{
	float s;
	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

static void init(void)
{
	int i, j, k;
	for (i = 0; i < B; i++) {
		p[i] = i;
		for (j = 0; j < 2; j++)
			g2[i][j] = (float)((rand() % (B + B)) - B) / B;
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}
	for (i = 0; i < B + 2; i++) {
		p[B + i] = p[i];
		for (j = 0; j < 2; j++)
			g2[B + i][j] = g2[i][j];
	}
}

void MySaveBmp()
{
	const char* filename = "texture.bmp";
	int width = 500;
	int height = 500;
	int byteNum = 3 * width * height;
	unsigned char* rgbbuf = new unsigned char[byteNum];
	float d1 = -300.0, d2 = -300.0;
	const float size = 0.1f;
	const int pix = width * size / 10;
	for (int i = 0; i < 100; i++) {
		d1 = -300;
		for (int j = 0; j < 100; j++) {
			float vec[2] = { d1,d2 };
			float color = (PerlinNoise(vec)*8 + 1) / 4;
			int GValue = (int)(color * 256);
			for (int tempy = 0; tempy <= 5; tempy++){
				for (int temp = 0; temp <= 5; temp++){
					rgbbuf[i * 15 + j * 7500 + temp * 3 + tempy * 1500] = 0;
					rgbbuf[i * 15 + j * 7500 + 1 + temp * 3 + tempy * 1500] = 0;
					rgbbuf[i * 15 + j * 7500 + 2 + temp * 3 + tempy * 1500] = GValue;
				}
			}
			d1 += 0.1;
		}
		d2 += 0.1;
	}


	MyBITMAPFILEHEADER bfh;
	MyBITMAPINFOHEADER bih;
	unsigned short bfType = 0x4d42;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfSize = 2 + sizeof(MyBITMAPFILEHEADER) + sizeof(MyBITMAPINFOHEADER) + width * height * 2;
	bfh.bfOffBits = 0x36;

	bih.biSize = sizeof(MyBITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 5000;
	bih.biYPelsPerMeter = 5000;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	printf("filename=%s \n", filename);
	FILE* file = fopen(filename, "wb");
	if (!file)
	{
		printf("Could not write file\n");
		return;
	}

	/*Write headers*/
	fwrite(&bfType, sizeof(bfType), 1, file);
	fwrite(&bfh, sizeof(bfh), 1, file);
	fwrite(&bih, sizeof(bih), 1, file);

	fwrite(rgbbuf, byteNum, 1, file);
	fclose(file);
}



#include <windows.h>  //Windows Header
#include <gl\gl.h>   //OpenGL Header
#include <gl\glu.h>   //GLu32 Header
#include <iostream>
#include <gl\glut.h>
#include <stdio.h>
#include "Math.h"
#include "Main.h"
#include "Readfile.h"
#include <vector>
#include <random>
#include <time.h>
#include "PerlinNoise.h"

using namespace std;

const int screenWidth = 500;
const int screenHeight = 500;
float finalCoordinate[10][3];
float newFinalCoordinate[10][3];
int pointnum = 0, planenum = 0, plane[7][7];
int isVisable[10];
Edge *pEdgeTable[screenWidth];
Edge *pNewEdgeTable[screenWidth];
Edge *pActiveEdgeTable;
Edge *pNewActiveEdgeTable;
float zBufferDepth[screenWidth][screenHeight];
int zBufferColorR[screenWidth][screenHeight];
int zBufferColorG[screenWidth][screenHeight];
int zBufferColorB[screenWidth][screenHeight];
float camera[3] = { 50, 30, 70 };
float lightPoint[3] = { 15,50,20 };
float kd = 0.2f;
float ks = 0.6f;
float ka = 0.5f;
int mode = 3; //mode=1 constant shading; mode=2 Gouraud shading; mode = 3 Phong shading
float lightColor[3] = { 150,150,150 };
float diffuseColor[3] = { 150,150,150 };
float ambientColor[3] = {100,0,0};
float cRotation[3] = { 0,1, 0 };
float refPoint[3] = { 0, 0, 30 };
float d = 10, f = 200, h = 10;
float mView[4][4];
float x[10], y[10], z[10];
float xNew[10], yNew[10], zNew[10];
float planeNormal[7][3];
float vertexNormal[10][3];
const int textureWidth = 500;
const int textureHeight = 500;
int textureR[textureWidth][textureHeight];
int textureG[textureWidth][textureHeight];
int textureB[textureWidth][textureHeight];

void render(void) {

	// calculate plane normal and vertex normal for gouraud and phone shading 
	if (mode == 2 || mode==3) {
		// calculate plane normal and normalize it.+
		for (int i = 0; i < planenum; i++)
		{
				planeNormal[i][0] = (y[plane[i][2] - 1] - y[plane[i][1] - 1]) * (z[plane[i][2] - 1] - z[plane[i][3] - 1]) - (z[plane[i][2] - 1] - z[plane[i][1] - 1]) * (y[plane[i][2] - 1] - y[plane[i][3] - 1]);
				planeNormal[i][1] = 0 - ( ((x[plane[i][2] - 1] - x[plane[i][1] - 1]) * (z[plane[i][2] - 1] - z[plane[i][3] - 1]) ) - (z[plane[i][2] - 1] - z[plane[i][1] - 1]) * (x[plane[i][2] - 1] - x[plane[i][3] - 1]));
				planeNormal[i][2] = (x[plane[i][2] - 1] - x[plane[i][1] - 1]) * (y[plane[i][2] - 1] - y[plane[i][3] - 1]) - (y[plane[i][2] - 1] - y[plane[i][1] - 1]) * (x[plane[i][2] - 1] - x[plane[i][3] - 1]);
				float normalizePlaneN = sqrt(planeNormal[i][0] * planeNormal[i][0] + planeNormal[i][1] * planeNormal[i][1] + planeNormal[i][2] * planeNormal[i][2]);
				planeNormal[i][0] = planeNormal[i][0] / normalizePlaneN;
				planeNormal[i][1] = planeNormal[i][1] / normalizePlaneN;
				planeNormal[i][2] = planeNormal[i][2] / normalizePlaneN;
				//cout << "面" << i << "法向量： "<<planeNormal[i][0] << " " << planeNormal[i][1] << " " << planeNormal[i][2] << endl;
		}

		//calculate every vertex's normal
		for (int i = 0; i < planenum; i++)
		{
				for (int j = 1; j < plane[i][0]+1; j++) {
					int temp = plane[i][j] - 1;
					vertexNormal[temp][0] += planeNormal[i][0];
					vertexNormal[temp][1] += planeNormal[i][1];
					vertexNormal[temp][2] += planeNormal[i][2];
			}
		}
		for (int i = 0; i < 10; i++)
		{
			if (vertexNormal[i][0] != 0 || vertexNormal[i][1] != 0 || vertexNormal[i][2] != 0) {
				float tempN = sqrt(vertexNormal[i][0] * vertexNormal[i][0] + vertexNormal[i][1] * vertexNormal[i][1] + vertexNormal[i][2] * vertexNormal[i][2]);
				vertexNormal[i][0] = vertexNormal[i][0] / tempN;
				vertexNormal[i][1] = vertexNormal[i][1] / tempN;
				vertexNormal[i][2] = vertexNormal[i][2] / tempN;
			}
			cout << "vN" << i << ": " << vertexNormal[i][0] << " " << vertexNormal[i][1] << " " << vertexNormal[i][2] << endl;
		}
	}
	//transfer to pixel
	for (int i = 0; i < sizeof(finalCoordinate)/sizeof(finalCoordinate[0]); i++)
	{
		finalCoordinate[i][0] = round(finalCoordinate[i][0] * (screenWidth / 2) + (screenWidth / 2));
		finalCoordinate[i][1] = round(finalCoordinate[i][1] * (screenHeight / 2) + (screenHeight / 2));
	}
	for (int i = 0; i < sizeof(finalCoordinate) / sizeof(finalCoordinate[0]); i++)
	{
		newFinalCoordinate[i][0] = round(newFinalCoordinate[i][0] * (screenWidth / 2) + (screenWidth / 2));
		newFinalCoordinate[i][1] = round(newFinalCoordinate[i][1] * (screenHeight / 2) + (screenHeight / 2));
	}

	for (int k = 0; k < planenum; k++)
	{
		if (isVisable[k] == 1){
			int cR = 0;
			int cG = 0;
			int cB = 0;
			//calculate plane equation so that can get Z values
			float p1[3], p2[3], p3[3];
			p1[0] = finalCoordinate[plane[k][1] - 1][0];
			p1[1] = finalCoordinate[plane[k][1] - 1][1];
			p1[2] = finalCoordinate[plane[k][1] - 1][2];
			p2[0] = finalCoordinate[plane[k][2] - 1][0];
			p2[1] = finalCoordinate[plane[k][2] - 1][1];
			p2[2] = finalCoordinate[plane[k][2] - 1][2];
			p3[0] = finalCoordinate[plane[k][3] - 1][0];
			p3[1] = finalCoordinate[plane[k][3] - 1][1];
			p3[2] = finalCoordinate[plane[k][3] - 1][2];
			float A = (p2[1] - p1[1])*(p3[2] - p2[2]) - (p2[2] - p1[2])*(p3[1] - p2[1]);
			float B = 0 - ((p2[0] - p1[0])*(p3[2] - p2[2]) - (p2[2] - p1[2])*(p3[0] - p2[0]));
			float C = (p2[0] - p1[0])*(p3[1] - p2[1]) - (p2[1] - p1[1])*(p3[0] - p2[0]);
			float D = -(A*p1[0] + B*p1[1] + C*p1[2]);

			//move points which are on the plane into vector, so make it easy to calculate
			vector<Point> vertices;
			for (int i = 1; i < plane[k][0] + 1; i++)
			{
				int temp = plane[k][i] - 1;
				vertices.push_back(Point(finalCoordinate[temp][0], finalCoordinate[temp][1],finalCoordinate[temp][2]));
			}

			// mode=1 constant shading
			if (mode == 1) {
				//calculate N of the plane
				float np[3];
				np[0] = (y[plane[k][2] - 1] - y[plane[k][1] - 1]) * (z[plane[k][2] - 1] - z[plane[k][3] - 1]) - (z[plane[k][2] - 1] - z[plane[k][1] - 1]) * (y[plane[k][2] - 1] - y[plane[k][3] - 1]);
				np[1] = 0 - (((x[plane[k][2] - 1] - x[plane[k][1] - 1]) * (z[plane[k][2] - 1] - z[plane[k][3] - 1])) - (z[plane[k][2] - 1] - z[plane[k][1] - 1]) * (x[plane[k][2] - 1] - x[plane[k][3] - 1]));
				np[2] = (x[plane[k][2] - 1] - x[plane[k][1] - 1]) * (y[plane[k][2] - 1] - y[plane[k][3] - 1]) - (y[plane[k][2] - 1] - y[plane[k][1] - 1]) * (x[plane[k][2] - 1] - x[plane[k][3] - 1]);
				float normalizeN = sqrt(np[0] * np[0] + np[1] * np[1] + np[2] * np[2]);
				np[0] = np[0] / normalizeN;
				np[1] = np[1] / normalizeN;
				np[2] = np[2] / normalizeN;

				//calculate the light vector L 
				float vLight[3];
				vLight[0] = lightPoint[0] - x[plane[k][2] - 1];
				vLight[1] = lightPoint[1] - y[plane[k][2] - 1];
				vLight[2] = lightPoint[2] - z[plane[k][2] - 1];
				float normalizeL = sqrt(vLight[0] * vLight[0] + vLight[1] * vLight[1] + vLight[2] * vLight[2]);
				for (int i = 0; i < 3; i++)
				{
					vLight[i] = vLight[i] / normalizeL;
				}

				//calculate the dirction to the camera V
				float vCamera[3];
				vCamera[0] = camera[0] - x[plane[k][2] - 1];
				vCamera[1] = camera[1] - y[plane[k][2] - 1];
				vCamera[2] = camera[2] - z[plane[k][2] - 1];
				float normalizeV = sqrt(vCamera[0] * vCamera[0] + vCamera[1] * vCamera[1] + vCamera[2] * vCamera[2]);
				for (int i = 0; i < 3; i++)
				{
					vCamera[i] = vCamera[i] / normalizeV;
				}

				//calculate  the ideal reflective direction R
				float vr[3];
				vr[0] = 2 * np[0] * (np[0] * vLight[0] + np[1] * vLight[1] + np[2] * vLight[2]) - vLight[0];
				vr[1] = 2 * np[1] * (np[0] * vLight[0] + np[1] * vLight[1] + np[2] * vLight[2]) - vLight[1];
				vr[2] = 2 * np[2] * (np[0] * vLight[0] + np[1] * vLight[1] + np[2] * vLight[2]) - vLight[2];
				float normalizeR = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);
				for (int i = 0; i < 3; i++)
				{
					vr[i] = vr[i] / normalizeR;
				}

				float productNL = np[0] * vLight[0] + np[1] * vLight[1] + np[2] * vLight[2];
				if (productNL < 0)
					productNL = 0;
				float productVR = vCamera[0] * vr[0] + vCamera[1] * vr[1] + vCamera[2] * vr[2];
				if (productVR < 0)
					productVR = 0;

				cR = 0;
				cG = 0;
				cB = 0;

				float Lsr = lightColor[0] * ks * pow(productVR, 10.f);
				if (Lsr > 255)
					cR += 255;
				else if (Lsr < 0)
					cR += 0;
				else
					cR += Lsr;
				float Lsg = lightColor[1] * ks * pow(productVR, 10.f);
				if (Lsg > 255)
					cG = 255;
				else if (Lsg < 0)
					cG = 0;
				else
					cG = Lsg;
				float Lsb = lightColor[2] * ks * pow(productVR, 10.f);
				if (Lsb > 255)
					cB = 255;
				else if (Lsb < 0)
					cB = 0;
				else
					cB = Lsb;

				float Ldr = diffuseColor[0] * kd * (productNL);
				if (Ldr > 255)
					cR += 255;
				else if (Ldr < 0)
					cR += 0;
				else
					cR += Ldr;
				float Ldg = diffuseColor[1] * kd * (productNL);
				if (Ldg > 255)
					cG += 255;
				else if (Ldg < 0)
					cG += 0;
				else
					cG += Ldg;
				float Ldb = diffuseColor[2] * kd * (productNL);
				if (Ldb > 255)
					cB += 255;
				else if (Ldb < 0)
					cB += 0;
				else
					cB += Ldb;

				cR += ambientColor[0] * ka;
				cG += ambientColor[1] * ka;
				cB += ambientColor[2] * ka;
				//cout <<"RGB:"<< cR <<" "<< cG <<" "<< cB << endl;
			}

			// mode = 2 Gouraud shading
			if (mode == 2) {
				for (unsigned int i = 0; i < vertices.size(); i++)
				{
					vertices[i].R = 0;
					vertices[i].G = 0;
					vertices[i].B = 0;

					//calculate the light vector L 
					float vLight[3];
					vLight[0] = lightPoint[0] - x[plane[k][i+1] - 1];
					vLight[1] = lightPoint[1] - y[plane[k][i+1] - 1];
					vLight[2] = lightPoint[2] - z[plane[k][i+1] - 1];
					float normalizeL = sqrt(vLight[0] * vLight[0] + vLight[1] * vLight[1] + vLight[2] * vLight[2]);
					for (int j = 0; j < 3; j++)
					{
						vLight[j] = vLight[j] / normalizeL;
					}

					//calculate the dirction to the camera V
					float vCamera[3];
					vCamera[0] = camera[0] - x[plane[k][i+1] - 1];
					vCamera[1] = camera[1] - y[plane[k][i+1] - 1];
					vCamera[2] = camera[2] - z[plane[k][i+1] - 1];
					float normalizeV = sqrt(vCamera[0] * vCamera[0] + vCamera[1] * vCamera[1] + vCamera[2] * vCamera[2]);
					for (int j = 0; j < 3; j++)
					{
						vCamera[j] = vCamera[j] / normalizeV;
					}

					//calculate  the ideal reflective direction R
					float vr[3];
					vr[0] = 2 * vertexNormal[plane[k][i + 1] - 1][0] * (vertexNormal[plane[k][i + 1] - 1][0] * vLight[0] + vertexNormal[plane[k][i + 1] - 1][1] * vLight[1] + vertexNormal[plane[k][i + 1] - 1][2] * vLight[2]) - vLight[0];
					vr[1] = 2 * vertexNormal[plane[k][i + 1] - 1][1] * (vertexNormal[plane[k][i + 1] - 1][0] * vLight[0] + vertexNormal[plane[k][i + 1] - 1][1] * vLight[1] + vertexNormal[plane[k][i + 1] - 1][2] * vLight[2]) - vLight[1];
					vr[2] = 2 * vertexNormal[plane[k][i + 1] - 1][2] * (vertexNormal[plane[k][i + 1] - 1][0] * vLight[0] + vertexNormal[plane[k][i + 1] - 1][1] * vLight[1] + vertexNormal[plane[k][i + 1] - 1][2] * vLight[2]) - vLight[2];
					float normalizeR = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);
					for (int j = 0; j < 3; j++)
					{
						vr[j] = vr[j] / normalizeR;
					}

					float productNL = vertexNormal[plane[k][i + 1] - 1][0] * vLight[0] + vertexNormal[plane[k][i + 1] - 1][1] * vLight[1] + vertexNormal[plane[k][i + 1] - 1][2] * vLight[2];
					if (productNL < 0)
						productNL = 0;
					float productVR = vCamera[0] * vr[0] + vCamera[1] * vr[1] + vCamera[2] * vr[2];
					if (productVR < 0)
						productVR = 0;

					float Lsr = lightColor[0] * ks * pow(productVR, 10.f);
					if (Lsr > 255)
						vertices[i].R += 255;
					else if (Lsr < 0)
						vertices[i].R += 0;
					else
						vertices[i].R += Lsr;
					float Lsg = lightColor[1] * ks * pow(productVR, 10.f);
					if (Lsg > 255)
						vertices[i].G += 255;
					else if (Lsg < 0)
						vertices[i].G += 0;
					else
						vertices[i].G += Lsg;
					float Lsb = lightColor[2] * ks * pow(productVR, 10.f);
					if (Lsb > 255)
						vertices[i].B += 255;
					else if (Lsb < 0)
						vertices[i].B += 0;
					else
						vertices[i].B += Lsb;

					float Ldr = diffuseColor[0] * kd * (productNL);
					if (Ldr > 255)
						vertices[i].R += 255;
					else if (Ldr < 0)
						vertices[i].R += 0;
					else
						vertices[i].R += Ldr;
					float Ldg = diffuseColor[1] * kd * (productNL);
					if (Ldg > 255)
						vertices[i].G += 255;
					else if (Ldg < 0)
						vertices[i].G += 0;
					else
						vertices[i].G += Ldg;
					float Ldb = diffuseColor[2] * kd * (productNL);
					if (Ldb > 255)
						vertices[i].B += 255;
					else if (Ldb < 0)
						vertices[i].B += 0;
					else
						vertices[i].B += Ldb;

					vertices[i].R += ambientColor[0] * ka;
					vertices[i].G += ambientColor[1] * ka;
					vertices[i].B += ambientColor[2] * ka;


				}
			}

			if (mode == 3) {
				for (unsigned int i = 0; i < vertices.size(); i++)
				{
					vertices[i].N0 = vertexNormal[plane[k][i + 1] - 1][0];
					vertices[i].N1 = vertexNormal[plane[k][i + 1] - 1][1];
					vertices[i].N2 = vertexNormal[plane[k][i + 1] - 1][2];
				}
			}
			
			//get maxY of the plane
			int maxY = 0;
			for (int i = 1; i < plane[k][0]+1; i++)
			{
				int temp = plane[k][i] - 1;
				if (finalCoordinate[temp][1] > maxY)
				{
					maxY = finalCoordinate[temp][1];
				}
			}

			pActiveEdgeTable = new Edge();
			pActiveEdgeTable->next = nullptr;

			glColor3ub(cR, cG, cB);
			glBegin(GL_POINTS);



			//get EdgeTable
			for (unsigned int i = 0; i < vertices.size(); i++)
			{
				//get current point and two points behind it and one point before it
				int x0 = vertices[(i - 1 + vertices.size()) % vertices.size()].x;
				int x1 = vertices[i].x;
				int x2 = vertices[(i + 1) % vertices.size()].x;
				int x3 = vertices[(i + 2) % vertices.size()].x;
				int y0 = vertices[(i - 1 + vertices.size()) % vertices.size()].y;
				int y1 = vertices[i].y;
				int y2 = vertices[(i + 1) % vertices.size()].y;
				int y3 = vertices[(i + 2) % vertices.size()].y;
				if (y1 == y2)
					continue;
				int ymin = y1 > y2 ? y2 : y1;
				int ymax = y1 > y2 ? y1 : y2;
				float x = y1 > y2 ? x2 : x1;
				float dx = (x1 - x2) * 1.0f / (y1 - y2);
				float R = y1 > y2 ? vertices[(i + 1) % vertices.size()].R : vertices[i].R;
				float G = y1 > y2 ? vertices[(i + 1) % vertices.size()].G : vertices[i].G;
				float B = y1 > y2 ? vertices[(i + 1) % vertices.size()].B : vertices[i].B;
				float dR = (vertices[i].R - vertices[(i + 1) % vertices.size()].R) / (y1 - y2);
				float dG = (vertices[i].G - vertices[(i + 1) % vertices.size()].G) / (y1 - y2);
				float dB = (vertices[i].B - vertices[(i + 1) % vertices.size()].B) / (y1 - y2);
				float dN0 = (vertices[i].N0 - vertices[(i + 1) % vertices.size()].N0) / (y1 - y2);
				float dN1 = (vertices[i].N1 - vertices[(i + 1) % vertices.size()].N1) / (y1 - y2);
				float dN2 = (vertices[i].N2 - vertices[(i + 1) % vertices.size()].N2) / (y1 - y2);
				float N0 = y1 > y2 ? vertices[(i + 1) % vertices.size()].N0 : vertices[i].N0;
				float N1 = y1 > y2 ? vertices[(i + 1) % vertices.size()].N1 : vertices[i].N1;
				float N2 = y1 > y2 ? vertices[(i + 1) % vertices.size()].N2 : vertices[i].N2;
				if (((y1 < y2) && (y1 > y0)) || ((y2 < y1) && (y2 > y3)))
				{
					ymin++;
					x += dx;
				}
				Edge *p = new Edge();
				p->R = R;
				p->G = G;
				p->B = B;
				p->dR = dR;
				p->dG = dG;
				p->dB = dB;
				p->dN0 = dN0;
				p->dN1 = dN1;
				p->dN2 = dN2;
				p->N0 = N0;
				p->N1 = N1;
				p->N2 = N2;
				p->ymax = ymax;
				p->x = x;
				p->dx = dx;
				p->next = pEdgeTable[ymin]->next;
				pEdgeTable[ymin]->next = p;
			}
			vertices.clear();
			
			//scan line from 0 to mayY
			for (int i = 0; i<maxY; i++)
			{
				
				while (pEdgeTable[i]->next)
				{
					Edge *pInsert = pEdgeTable[i]->next;
					Edge *p = pActiveEdgeTable;
					while (p->next)
					{
						if (pInsert->x > p->next->x)
						{
							p = p->next;
							continue;
						}
						if (pInsert->x == p->next->x && pInsert->dx > p->next->dx)
						{
							p = p->next;
							continue;
						}
						break;
					}
					pEdgeTable[i]->next = pInsert->next;
					pInsert->next = p->next;
					p->next = pInsert;
				}

				Edge *p = pActiveEdgeTable;
				while (p->next && p->next->next)
				{
					for (int xCoordinate = p->next->x; xCoordinate < p->next->next->x; xCoordinate++)
					{
						float zCoordinate = -(A*xCoordinate + B*i + D) / C;
						float N0,N1,N2;
						if (zCoordinate < zBufferDepth[xCoordinate][i]){
							if (mode == 2) {
								cR = p->next->R * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									p->next->next->R * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								cG = p->next->G * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									p->next->next->G * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								cB = p->next->B * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									p->next->next->B * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								if (cR > 255)
									cR = 255;
								if (cR < 0)
									cR = 0;
								if (cG > 255)
									cG = 255;
								if (cG < 0)
									cG = 0;
								if (cB > 255)
									cB = 255;
								if (cB < 0)
									cB = 0;
								//cout << cR << " " << cG << " " << cB << endl;
								glColor3ub(cR, cG, cB);
							}

							if (mode == 3) {
								 N0 = p->next->N0 * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									p->next->next->N0 * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								 N1 = p->next->N1 * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									 p->next->next->N1 * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								 N2 = p->next->N2 * (p->next->next->x - xCoordinate) / (p->next->next->x - p->next->x) +
									 p->next->next->N2 * (xCoordinate - p->next->x) / (p->next->next->x - p->next->x);
								 float normalizeN = sqrt(N0 * N0 + N1 * N1 + N2 * N2);
								 N0 = N0 / normalizeN;
								 N1 = N1 / normalizeN;
								 N2 = N2 / normalizeN;

								 //calculate the light vector L 
								 float vLight[3];
								 vLight[0] = lightPoint[0] - refPoint[0]; 
								 vLight[1] = lightPoint[1] - refPoint[1];
								 vLight[2] = lightPoint[2] - refPoint[2];
								 float normalizeL = sqrt(vLight[0] * vLight[0] + vLight[1] * vLight[1] + vLight[2] * vLight[2]);
								 for (int i = 0; i < 3; i++)
								 {
									 vLight[i] = vLight[i] / normalizeL;
								 }

								 //calculate the dirction to the camera V
								 float vCamera[3];
								 vCamera[0] = camera[0] - refPoint[0];
								 vCamera[1] = camera[1] - refPoint[1];
								 vCamera[2] = camera[2] - refPoint[2];
								 float normalizeV = sqrt(vCamera[0] * vCamera[0] + vCamera[1] * vCamera[1] + vCamera[2] * vCamera[2]);
								 for (int i = 0; i < 3; i++)
								 {
									 vCamera[i] = vCamera[i] / normalizeV;
								 }

								 //calculate  the ideal reflective direction R
								 float vr[3];
								 vr[0] = 2 * N0 * (N0 * vLight[0] + N1 * vLight[1] + N2 * vLight[2]) - vLight[0];
								 vr[1] = 2 * N1 * (N0 * vLight[0] + N1 * vLight[1] + N2 * vLight[2]) - vLight[1];
								 vr[2] = 2 * N2 * (N0 * vLight[0] + N1 * vLight[1] + N2 * vLight[2]) - vLight[2];
								 float normalizeR = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);
								 for (int i = 0; i < 3; i++)
								 {
									 vr[i] = vr[i] / normalizeR;
								 }

								 float productNL = N0 * vLight[0] + N1 * vLight[1] + N2 * vLight[2];
								 if (productNL < 0)
									 productNL = 0;
								 float productVR = vCamera[0] * vr[0] + vCamera[1] * vr[1] + vCamera[2] * vr[2];
								 if (productVR < 0)
									 productVR = 0;
								 cR = 0;
								 cG = 0;
								 cB = 0;

								 float Lsr = lightColor[0] * ks * pow(productVR, 10.f);
								 if (Lsr> 255)
									 cR += 255;
								 else if (Lsr < 0)
									 cR += 0;
								 else
									 cR += Lsr;
								 float Lsg = lightColor[1] * ks * pow(productVR, 10.f);
								 if (Lsg >255)
									 cG = 255;
								 else if (Lsg < 0)
									 cG = 0;
								 else
									 cG = Lsg;	
								 float Lsb = lightColor[2] * ks * pow(productVR, 10.f);
								 if (Lsb >255)
									 cB = 255;
								 else if (Lsb < 0)
									 cB = 0;
								 else
									 cB = Lsb;

								 float Ldr = diffuseColor[0] * kd * (productNL);
								 if (Ldr > 255)
									 cR += 255;
								 else if (Ldr < 0)
									 cR += 0;
								 else
									 cR += Ldr;
								 float Ldg = diffuseColor[1] * kd * (productNL);
								 if (Ldg > 255)
									 cG += 255;
								 else if (Ldg < 0)
									 cG += 0;
								 else
									 cG += Ldg;
								 float Ldb = diffuseColor[2] * kd * (productNL);
								 if (Ldb > 255)
									 cB += 255;
								 else if (Ldb < 0)
									 cB += 0;
								 else
									 cB += Ldb;
								
								//cR += ambientColor[0] * ka;
								// cG += ambientColor[1] * ka;
								// cB += ambientColor[2] * ka;
								 int vC = zCoordinate * screenWidth;
								 cR += textureR[i][vC] * ka;
								 cG += textureG[i][vC] * ka;
								 cB += textureB[i][vC] * ka;
							
								 glColor3ub(cR, cG, cB);
							}

							glVertex2i(xCoordinate, i);
							zBufferDepth[xCoordinate][i] = zCoordinate;
							zBufferColorB[xCoordinate][i] = cB;
							zBufferColorG[xCoordinate][i] = cG;
							zBufferColorR[xCoordinate][i] = cR;
						}
					}
					p = p->next->next;
				}

				p = pActiveEdgeTable;
				while (p->next)
				{
					if (p->next->ymax == i)
					{
						Edge *pDelete = p->next;
						p->next = pDelete->next;
						pDelete->next = nullptr;
						delete pDelete;
					}
					else
					{
						p = p->next;
					}
				}

				p = pActiveEdgeTable;
				while (p->next)
				{
					p->next->x += p->next->dx;
					p->next->R += p->next->dR;
					p->next->G += p->next->dG;
					p->next->B += p->next->dB;
					p->next->N0 += p->next->dN0;
					p->next->N1 += p->next->dN1;
					p->next->N2 += p->next->dN2;
					p = p->next;
				}

			}
			glEnd();
			glFlush();
			delete pActiveEdgeTable;
		}
	}
}

void initializeZBuffer(){
	for (int i = 0; i < screenHeight; i++){
		pEdgeTable[i] = new Edge();
		pNewEdgeTable[i] = new Edge();
		pEdgeTable[i]->next = nullptr;
		pNewEdgeTable[i]->next = nullptr;
		for (int j = 0; j < screenWidth; j++)
		{
			zBufferColorR[j][i]= 0;
			zBufferColorG[j][i] = 0;
			zBufferColorB[j][i] = 0;
			zBufferDepth[j][i] = FLT_MAX;
		}
	}
}


void main(int argc, char *argv[]) {
	
	MySaveBmp();

	//read file method
	read(x, y, z, pointnum, planenum, plane);
	readTexture(textureR,textureG,textureB);
	//calculate R and T then get MView
	calculateMview(camera, cRotation, refPoint, mView);
	//calculate every plane then diside draw it or not
	backCulling(x, y, z, isVisable, camera, planenum, plane, mView);

	//calculate MPers then Mutiply it with Mview, and multiple [x,y,z,1] to get the final coordinate on the view plane
	transform(x, y, z, mView, finalCoordinate, d, f, h);

	glutInit(&argc, argv);
	// set windows size
	glutInitWindowSize(screenWidth, screenHeight);
	// set OpenGL display mode 
	glutInitDisplayMode(GLUT_RGBA);
	// set window title
	glutCreateWindow("Lab-5 By. Bingxin Lyu");
	// set rendering function
	glMatrixMode(GL_PROJECTION);
	initializeZBuffer();
	gluOrtho2D(0.0, screenHeight, 0.0, screenWidth);
	glutDisplayFunc(render);
	glutMainLoop();
	
	
}

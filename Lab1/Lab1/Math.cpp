#include <math.h>
#include <iostream>

using namespace std;

void calculateMview(float(&camera)[3], float(&cRotation)[3], float(&refPoint)[3], float(&mView)[4][4]){

	// calculate R and T
	float mT[4][4] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0, 1 } };
	float mR[4][4];
	float normal = 1;
	for (int i = 0; i < 3; i++)
	{
		mT[i][3] = 0 - camera[i];
		if ((refPoint[i] - camera[i]) == 0)
			mR[2][i] = 0.f;
		else
			mR[2][i] = (refPoint[i] - camera[i]) / normal;
		mR[i][3] = 0.f;
		mR[3][i] = 0.f;
	}
	normal = sqrt(pow(mR[2][0], 2) + pow(mR[2][1], 2) + pow(mR[2][2], 2));
	mR[2][0] = mR[2][0] / normal;
	mR[2][1] = mR[2][1] / normal;
	mR[2][2] = mR[2][2] / normal;

	mR[0][0] = (mR[2][1] * cRotation[2]) - (mR[2][2] * cRotation[1]);
	mR[0][1] = 0-((mR[2][0] * cRotation[2]) - (mR[2][2] * cRotation[0]));
	mR[0][2] = (mR[2][0] * cRotation[1]) - (mR[2][1] * cRotation[0]);
	normal = sqrt(pow(mR[0][0], 2) + pow(mR[0][1], 2) + pow(mR[0][2], 2));
	mR[0][0] = mR[0][0] / normal;
	mR[0][1] = mR[0][1] / normal;
	mR[0][2] = mR[0][2] / normal;

	mR[1][0] = (mR[0][1] * mR[2][2]) - (mR[0][2] * mR[2][1]);
	mR[1][1] = 0-((mR[0][0] * mR[2][2]) - (mR[0][2] * mR[2][0]));
	mR[1][2] = (mR[0][0] * mR[2][1]) - (mR[0][1] * mR[2][0]);
	mR[3][3] = 1.f;

	//cout << "-------------------" << endl;
	//cout << "mR" << endl;
	//for (int i = 0; i < 4; i++){
	//	for (int j = 0; j < 4; j++){
	//		cout << mR[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << "-------------------" << endl;
	//cout << "mT" << endl;
	//for (int i = 0; i < 4; i++){
	//	for (int j = 0; j < 4; j++){
	//		cout << mT[i][j] << " ";
	//	}
	//	cout << endl;
	//}

	//Calculate Mview by R * T
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			mView[i][j] = 0.f;
			for (int k = 0; k < 4; k++){
				mView[i][j] += mR[i][k] * mT[k][j];
			}
		}
	}



}


void backCulling(float x[10], float y[10], float z[10], int(&isVisable)[10], float camera[3], int planenum, int plane[7][7],float mView[4][4]){
	float coordinateInCamera[10][3];
	for (int i = 0; i < 10; i++)
	{
		coordinateInCamera[i][0] = 0.f;
		coordinateInCamera[i][1] = 0.f;
		coordinateInCamera[i][2] = 0.f;
		float w = mView[3][0] * x[i] + mView[3][1] * y[i] + mView[3][2] * z[i] + mView[3][3];
		coordinateInCamera[i][0] = (mView[0][0] * x[i] + mView[0][1] * y[i] + mView[0][2] * z[i] + mView[0][3])/w;
		coordinateInCamera[i][1] = (mView[1][0] * x[i] + mView[1][1] * y[i] + mView[1][2] * z[i] + mView[1][3]) / w;
		coordinateInCamera[i][2] = (mView[2][0] * x[i] + mView[2][1] * y[i] + mView[2][2] * z[i] + mView[2][3]) / w;
	}

	for (int i = 0; i < planenum; i++)
	{
		float x1[3], x2[3], x3[3];
		int temp = plane[i][1];
		x1[0] = coordinateInCamera[temp-1][0];
		x1[1] = coordinateInCamera[temp - 1][1];
		x1[2] = coordinateInCamera[temp - 1][2];
		temp = plane[i][2];
		x2[0] = coordinateInCamera[temp - 1][0];
		x2[1] = coordinateInCamera[temp - 1][1];
		x2[2] = coordinateInCamera[temp - 1][2];
		temp = plane[i][3];
		x3[0] = coordinateInCamera[temp - 1][0];
		x3[1] = coordinateInCamera[temp - 1][1];
		x3[2] = coordinateInCamera[temp - 1][2];
		float np[3];
		np[0] = (x2[1] - x1[1])*(x3[2] - x2[2]) - (x2[2] - x1[2])*(x3[1] - x2[1]);
		np[1] = 0 - ((x2[0] - x1[0])*(x3[2] - x2[2]) - (x2[2] - x1[2])*(x3[0] - x2[0]));
		np[2] = (x2[0] - x1[0])*(x3[1] - x2[1]) - (x2[1] - x1[1])*(x3[0] - x2[0]);
		float ns[3];
		ns[0] = 0 - x2[0];
		ns[1] = 0 - x2[1];
		ns[2] = 0 - x2[2];
		float result = np[0] * ns[0] + np[1] * ns[1] + np[2] * ns[2];
		if (result > 0)
		{
			isVisable[i] = 1;
		}
		else
			isVisable[i] = 0;
		//cout << i << ":" << isVisable[i] <<" "<<result<< endl;
	}
}

void transform(float(&x)[10], float(&y)[10], float(&z)[10], float(&mView)[4][4], float(&finnalCoordinate)[10][3], float d, float f, float h){
	
	float mPer[4][4] = { { d / h, 0, 0, 0 }, { 0, d / h, 0, 0 }, { 0, 0, f / (f - d), -(d*f) / (f - d) }, { 0, 0, 1, 0 } };
	
	//multiple Mper and MView, save the martix as Mresult
	float mResult[4][4];
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			mResult[i][j] = 0.f;
			for (int k = 0; k < 4; k++){
				mResult[i][j] += mPer[i][k] * mView[k][j];
			}
		}
	}

	//Mutiple Mresult with every xyzw, so get every point's coodinate on the view plane
	for (int i = 0; i < (sizeof(x) / sizeof(float)); i++)
	{
		finnalCoordinate[i][0] = 0.f;
		finnalCoordinate[i][1] = 0.f;
		finnalCoordinate[i][2] = 0.f;
		float w = mResult[3][0] * x[i] + mResult[3][1] * y[i] + mResult[3][2] * z[i] + mResult[3][3];
		finnalCoordinate[i][0] = (mResult[0][0] * x[i] + mResult[0][1] * y[i] + mResult[0][2] * z[i] + mResult[0][3]) / w;
		finnalCoordinate[i][1] = (mResult[1][0] * x[i] + mResult[1][1] * y[i] + mResult[1][2] * z[i] + mResult[1][3]) / w;
		finnalCoordinate[i][2] = (mResult[2][0] * x[i] + mResult[2][1] * y[i] + mResult[2][2] * z[i] + mResult[2][3]) / w;
	}

	//These prints are use to test the result
	//cout << "-------------------" << endl;
	//cout << "mPer" << endl;
	//for (int i = 0; i < 4; i++){
	//	for (int j = 0; j < 4; j++){
	//		cout << mPer[i][j] << " ";
	//	}
	//	cout << endl;
	//}

	//cout << "-------------------" << endl;
	//cout << "mView" << endl;
	//for (int i = 0; i < 4; i++){
	//	for (int j = 0; j < 4; j++){
	//		cout << mView[i][j] << " ";
	//	}
	//	cout << endl;
	//}

	//cout << "-------------------" << endl;
	//cout << "Mresult" << endl;
	//for (int i = 0; i < 4; i++){
	//	for (int j = 0; j < 4; j++){
	//		cout << mResult[i][j] << " ";
	//	}
	//	cout << endl;
	//}

	//cout << "-------------------" << endl;
	//cout << "FinnalCoordinate" << endl;
	//for (int i = 0; i < 10; i++)
	//{
	//	for (int j = 0; j < 3; j++)
	//	{
	//		cout << finnalCoordinate[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << "-------------------" << endl;


}


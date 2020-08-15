#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <vector>

using namespace std;
ifstream bmpfile;
const int width = 500;
const int height = 500;

struct BITMAPFILEHEADERNEW {
	char bfType[2]; // must be "BM"
	char bfSize[4]; // the size of the bmp file
	char bfReserved1[2];
	char bfReserved2[2];
	char bfOffBits[4]; // the offset to the bitmap data
} FileHeader;

struct BITMAPINFOHEADERNEW {
	char biSize[4];        // the size of BITMAPINFOHEADER
	char biWidth[4];       // width (pixels)
	char biHeight[4];      // height (pixels)
	char biPlanes[2];      // color planes
	char biBitCount[2];    // bits per pixel
	char biCompression[4]; // type of compression (0 is no compression)
	char
		biSizeImage[4]; // the origin size of the bitmap data (before compression)
	char biXPelsPerMeter[4]; // horizontal pixels per meter
	char biYPelsPerMeter[4]; // vertical pixels per meter
	char biClrUsed[4];       // the number of colors used
	char biClrImportant[4];  // "important" colors, usually 0
} InfoHeader;

struct Pixel {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	// unsigned char alpha;
} p;

void read(float(&x)[10], float(&y)[10], float(&z)[10], int &pointnum, int &planenum, int(&plane)[7][7]){
	string fname = "house.d";
	string line;
	char data[1024];

	//open house.d
	ifstream infile;
	infile.open(fname);

	//Get number of points
	infile >> data;
	infile >> pointnum;

	//Get number of planes
	infile >> planenum;

	//Get vector of every points
	for (int i = 0; i < pointnum; i++)
	{
		infile >> x[i];
		infile >> y[i];
		infile >> z[i];
	}
	//Get point's infomation of plane
	for (int i = 0; i < planenum; i++)
	{
		int linenum;
		infile >> linenum;
		plane[i][0] = (float)linenum;
		for (int j = 1; j <= linenum; j++)
		{
			infile >> plane[i][j];
		}
	}



	infile.close();
}

inline int ToHumanRead(char* str, int size = 4) {
	// Convert to Big Endian
	long l = 0;
	memcpy(&l, str, size);
	return (signed int)l;
}

void drop_alpha() {
	if (ToHumanRead(InfoHeader.biBitCount) == 32) {
		unsigned char null;
		bmpfile.read((char*)&null, sizeof(null));
	}
}



vector<Pixel>* ReadBitmap() {
	int offset = 0;
	int height = ToHumanRead(InfoHeader.biHeight);
	int width = ToHumanRead(InfoHeader.biWidth);

	int linebyte = width * ToHumanRead(InfoHeader.biBitCount) / 8;
	offset = linebyte % 4;
	if (offset != 0)
		offset = 4 - offset;
	//cout << "Offset: " << offset << endl;

	vector<Pixel>* vec = new vector<Pixel>[abs(height)];

	bool isBottom = false;
	if (ToHumanRead(InfoHeader.biHeight) > 0) // read from bottom
	{
		isBottom = true;
	}
	for (int i = 0; i < abs(height); i++) {
		for (int j = 0; j < width; j++) {
			bmpfile.read((char*)&p, sizeof(p));
			if (!isBottom)
				vec[i].push_back(p);
			else
				vec[height - i - 1].push_back(p);
			drop_alpha();
		}
		for (int j = 0; j < offset; j++) {
			unsigned char null;
			bmpfile.read((char*)&null, sizeof(null));
		}
	}
	return vec;
}



void readTexture(int(&textureR)[width][height], int(&textureG)[width][height], int(&textureB)[width][height]) {
	bmpfile.open("texture.bmp", ios::in | ios::binary);
	if (bmpfile.is_open()) {
		bmpfile.read((char*)&FileHeader, sizeof(FileHeader));
		if (strncmp(FileHeader.bfType, "BM", 2) != 0) {
			cout << "Not a BMP File, or an Unsupported OS/2 BMP File." << endl;
		}
		bmpfile.read((char*)&InfoHeader, sizeof(InfoHeader));

		vector<Pixel>* vec = ReadBitmap();
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				textureR[i][j] = vec[i][j].r;
				textureG[i][j] = vec[i][j].g;
				textureB[i][j] = vec[i][j].b;
			}
		}
		//cout << textureR[511][511] <<" "<< textureG[511][511] << " " << textureB[511][511] << endl;
		bmpfile.close();
		delete[] vec;
	}
	else {
		cout << "Open File Error." << endl;
	}
}
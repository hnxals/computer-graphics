class Edge{
public:
	int ymax;
	float x;
	float dx;
	float R;
	float dR;
	float G;
	float dG;
	float B;
	float dB;
	float dN0;
	float dN1;
	float dN2;
	float N0;
	float N1;
	float N2;
	Edge* next;
};

class Point{
public:
	int x;
	int y;
	float z;
	int R;
	int G;
	int B;
	float N0;
	float N1;
	float N2;
	Point(int x, int y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};


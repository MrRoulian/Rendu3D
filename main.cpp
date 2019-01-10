#include "tgaimage.h"
#include "Point.h"
#include <iostream>
#include <string>
#include <sstream> 
#include <cstdlib>
#include <fstream>
#include <vector>
#include <time.h>


using namespace std;


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const int taille = 15000;


void drawPoint(int x0, int y0, TGAImage &image, TGAColor color) {
	image.set(x0, y0, color);
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { // if the line is steep, we transpose the image 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { // make it left−to−right 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    for (int x=x0; x<=x1; x++) { 
        float t = (x-x0)/(float)(x1-x0); 
        int y = y0*(1.-t) + y1*t; 
        if (steep) { 
            image.set(y, x, color); // if transposed, de−transpose 
        } else { 
            image.set(x, y, color); 
        } 
    } 
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	line(x0, y0, x1, y1, image, color);
	line(x1, y1, x2, y2, image, color);
	line(x2, y2, x0, y0, image, color);
}

void drawTriangle(Point p1, Point p2, Point p3, TGAImage &image, TGAColor color) {
	line(p1.x, p1.y, p2.x, p2.y, image, color);
	line(p2.x, p2.y, p3.x, p3.y, image, color);
	line(p3.x, p3.y, p1.x, p1.y, image, color);
}

int estAudessus(float a, float b, Point p) {
	float res = a * p.x + b;

	if (res < p.y) {
		return 1;
	}
	else {
		return -1;
	}
}

int min(int tab[], int nbVal) {
	if (nbVal == 0) { return 0; }
	int min = tab[0];
	for (int i = 1; i < nbVal; i++) {
		if (tab[i] < min) {
			min = tab[i];
		}
	}
	return min;
}

int max(int tab[], int nbVal) {
	if (nbVal == 0) { return 0; }
	int max = tab[0];
	for (int i = 1; i < nbVal; i++) {
		if (tab[i] > max) {
			max = tab[i];
		}
	}
	return max;
}

void drawFillTriangle(Point p1, Point p2, Point p3, TGAImage &image, TGAColor color) {
	drawTriangle(p1, p2, p3, image, color);
	int tabX[3]; tabX[0] = p1.x; tabX[1] = p2.x; tabX[2] = p3.x;
	int tabY[3]; tabY[0] = p1.y; tabY[1] = p2.y; tabY[2] = p3.y;

	int xMin = min(tabX, 3);
	int xMax = max(tabX, 3);
	int yMin = min(tabY, 3);
	int yMax = max(tabY, 3);

	int compteur = 0;
	float a1 = (p2.y+0.0 - p1.y+0.0) / (p2.x+0.0 - p1.x+0.0);
	float b1 = -a1 * p1.x+0.0 + p1.y+0.0;
	if (p2.x == p1.x) {
		a1 = 0;
		b1 = 0;
	}
	int res = estAudessus(a1, b1, p3);

	float a2 = (p3.y+0.0 - p2.y+0.0) / (p3.x+0.0 - p2.x+0.0);
	float b2 = -a2 * p2.x+0.0 + p2.y+0.0;
	if (p3.x == p2.x) {
		a2 = 0;
		b2 = 0;
	}
	int res1 = estAudessus(a2, b2, p1);

	float a3 = (p1.y+0.0 - p3.y+0.0) / (p1.x+0.0 - p3.x+0.0);
	float b3 = -a3 * p3.x+0.0 + p3.y+0.0;
	if (p1.x == p3.x) {
		a3 = 0;
		b3 = 0;
	}
	int res2 = estAudessus(a3, b3, p2);

	bool isOnTheSameLine = (a1 == a2 && a1 == a3) && (b1 == b2 && b1 == b3);
	if (isOnTheSameLine) {
		return;
	}
	
	for (int i = xMin; i < xMax; i++) {
		for (int j = yMin; j < yMax; j++) {
			if (res * j >= res * (a1 * i + b1) && 
				res1 * j >= res1 * (a2 * i + b2) && 
				res2 * j >= res2 * (a3 * i + b3)) {
				image.set(i, j, color);
			}
		}
	}

	//Sert pour le débug
	/*line(0, b1, 1000, a1 * 1000 + b1, image, res == 1 ? red : white);
	line(0, b2, 1000, a2 * 1000 + b2, image, res1 == 1 ? red : white);
	line(0, b3, 1000, a3 * 1000 + b3, image, res2 == 1 ? red : white);*/
}

TGAColor calcColor(double interval, double value) {
	TGAColor color;
	int x = (int)((1530 / interval)*value);
	int r = 0;
	int g = 0;
	int b = 0;
	if (x >= 0 && x < 255) {
		r = 255;
		g = x;
		b = 0;
	}
	if (x >= 255 && x < 510) {
		r = 510 - x;
		g = 255;
		b = 0;
	}
	if (x >= 510 && x < 765) {
		r = 0;
		g = 255;
		b = x - 510;
	}
	if (x >= 765 && x < 1020) {
		r = 0;
		g = 1020 - x;
		b = 255;
	}
	if (x >= 1020 && x < 1275) {
		r = x - 1020;
		g = 0;
		b = 255;
	}
	if (x >= 1275 && x <= 1530) {
		r = 255;
		g = 0;
		b = 1530 - x;
	}
	color = TGAColor(r, g, b, 255);
	return color;
}

void drawFile(string fileName, TGAImage &image, TGAColor color) {

	ifstream fichier(fileName, ios::in);  // on ouvre le fichier en lecture

	if (fichier)  // si l'ouverture a réussi
	{
		string option;
		string ligne;
		string trash;
		float x, y, z;
		int p1, p2, p3;
		int indice = 0;
		int indiceTriangle = 0;
		Point *tabPoint = new Point[6000];
		Point *tabTriangle = new Point[6000];
		Point p;

		while (getline(fichier, ligne)) {
			if (ligne[0] == 'v' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> x >> y >> z;
				x = x * (taille / 2) + taille / 2;
				y = y * (taille / 2) + taille / 2;
				z = z * (taille / 2) + taille / 2;
				p = Point(x, y, z);
				tabPoint[indice] = p;
				indice++;
				//drawPoint(x, y, image, red);
			}
			else if (ligne[0] == 'f' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> p1 >> trash >> p2 >> trash >> p3;
				tabTriangle[indiceTriangle] = Point(p1, p2, p3);
				indiceTriangle++;
			}
		}

		for (int i = 0; i < indiceTriangle-1 ; i++) {
			drawFillTriangle(tabPoint[tabTriangle[i].x-1], 
					tabPoint[tabTriangle[i].y-1], 
					tabPoint[tabTriangle[i].z-1], 
					image, calcColor(indiceTriangle,i));
		}

		fichier.close();  // on ferme le fichier
	}
	else {
		cout << "echec ouverture fichier" << endl;
	}
}

int main(int argc, char** argv) {
    TGAImage image(taille, taille, TGAImage::RGB);
	drawFile("diablo3_pose.txt", image, white);
	//drawFile("black_head.txt", image, white);
	//drawFile("bogie_head.txt", image, white);
	//drawFillTriangle(Point(10, 10, 10), Point(100, 20, 0), Point(100, 100, 0), image, white);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}

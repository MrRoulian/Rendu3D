#include "tgaimage.h"
#include "Point3D.h"
#include <iostream>
#include <string>
#include <sstream> 
#include <cstdlib>
#include <fstream>
#include <vector>
#include <time.h>
#include <D2d1_1helper.h>


using namespace std;


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const int taille = 2000;


#pragma region Math

D2D1_VECTOR_3F produitVectoriel (D2D1_VECTOR_3F vec1, D2D1_VECTOR_3F vec2) {
	D2D1_VECTOR_3F res;

	res.x = vec1.y*vec2.z - vec1.z*vec2.y;
	res.y = vec1.z*vec2.x - vec1.x*vec2.z;
	res.z = vec1.x*vec2.y - vec1.y*vec2.x;

	return res;
}

D2D1_VECTOR_3F getNormal(Point3D vec1, Point3D vec2, Point3D vec3) {
	D2D1_VECTOR_3F res;

	res.x = (vec2.yf - vec1.yf)*(vec3.zf - vec1.zf) - (vec2.zf - vec1.zf)*(vec3.yf - vec1.yf);
	res.y = (vec2.zf - vec1.zf)*(vec3.xf - vec1.xf) - (vec2.xf - vec1.xf)*(vec3.zf - vec1.zf);
	res.z = (vec2.xf - vec1.xf)*(vec3.yf - vec1.yf) - (vec2.yf - vec1.yf)*(vec3.xf - vec1.xf);

	return res;
}

float produitScalaire(D2D1_VECTOR_3F vec1, D2D1_VECTOR_3F vec2) {
	float res;

	res = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;

	return res;
}

int minTab(vector<int> tab) {
	if (tab.size() == 0) { return 0; }
	int min = tab[0];
	for (int i = 1; i < tab.size(); i++) {
		if (tab[i] < min) {
			min = tab[i];
		}
	}
	return min;
}

int maxTab(vector<int> tab) {
	if (tab.size() == 0) { return 0; }
	int max = tab[0];
	for (int i = 1; i < tab.size(); i++) {
		if (tab[i] > max) {
			max = tab[i];
		}
	}
	return max;
}

int estAudessus(float a, float b, Point3D p) {
	float res = a * p.x + b;

	if (res < p.y) {
		return 1;
	}
	else {
		return -1;
	}
}

#pragma endregion

#pragma region Outils

TGAColor calcColor(double interval, double value, float intensity) {
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
	color = TGAColor(intensity*r, intensity*g, intensity*b, 255);
	return color;
}

#pragma endregion

#pragma region Dessin

void drawPoint(int x0, int y0, TGAImage &image, TGAColor color) {
	image.set(x0, y0, color);
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	line(x0, y0, x1, y1, image, color);
	line(x1, y1, x2, y2, image, color);
	line(x2, y2, x0, y0, image, color);
}

void drawTriangle(Point3D p1, Point3D p2, Point3D p3, TGAImage &image, TGAColor color) {
	line(p1.x, p1.y, p2.x, p2.y, image, color);
	line(p2.x, p2.y, p3.x, p3.y, image, color);
	line(p3.x, p3.y, p1.x, p1.y, image, color);
}

void drawFillTriangle(Point3D p1, Point3D p2, Point3D p3, TGAImage &image, TGAColor color) {
	drawTriangle(p1, p2, p3, image, color);
	//Varialbe qui servent pour l'opti
	vector<int> tabX = vector<int>(); tabX.push_back(p1.x); tabX.push_back(p2.x); tabX.push_back(p3.x);
	vector<int> tabY = vector<int>(); tabY.push_back(p1.y); tabY.push_back(p2.y); tabY.push_back(p3.y);

	int xMin = minTab(tabX);
	int xMax = maxTab(tabX);
	int yMin = minTab(tabY);
	int yMax = maxTab(tabY);

	/*D2D1_VECTOR_3F abc;
	abc = produitVectoriel(p1,p2);*/

	//Je détermine les 3 fonctions affines du triangle et le sens dans le quel je dois remplir (a gauche de la courbe ou a droite)
	float a1 = (p2.y + 0.0 - p1.y + 0.0) / (p2.x + 0.0 - p1.x + 0.0);
	float b1 = -a1 * p1.x + 0.0 + p1.y + 0.0;
	if (p2.x == p1.x) {
		a1 = 0;
		b1 = 0;
	}
	int res = estAudessus(a1, b1, p3);

	float a2 = (p3.y + 0.0 - p2.y + 0.0) / (p3.x + 0.0 - p2.x + 0.0);
	float b2 = -a2 * p2.x + 0.0 + p2.y + 0.0;
	if (p3.x == p2.x) {
		a2 = 0;
		b2 = 0;
	}
	int res1 = estAudessus(a2, b2, p1);

	float a3 = (p1.y + 0.0 - p3.y + 0.0) / (p1.x + 0.0 - p3.x + 0.0);
	float b3 = -a3 * p3.x + 0.0 + p3.y + 0.0;
	if (p1.x == p3.x) {
		a3 = 0;
		b3 = 0;
	}
	int res2 = estAudessus(a3, b3, p2);

	//Vérification si les 3 points sont sur la même ligne, si c'est le cas on arrête la fonctionne car on à déjà dessiné le triangle vide avant
	bool isOnTheSameLine = (a1 == a2 && a1 == a3) && (b1 == b2 && b1 == b3);
	if (isOnTheSameLine) {
		return;
	}

	//Pour chaque pixel qui pourrait être dans le triangle on vérifie si il est dedans et si oui on le colorie
	for (int i = xMin; i < xMax; i++) {
		for (int j = yMin; j < yMax; j++) {
			if (res * j >= res * (a1 * i + b1) && res1 * j >= res1 * (a2 * i + b2) && res2 * j >= res2 * (a3 * i + b3)) {
				image.set(i, j, color);
			}
		}
	}

	//Sert pour le débug
	/*line(0, b1, 1000, a1 * 1000 + b1, image, res == 1 ? red : white);
	line(0, b2, 1000, a2 * 1000 + b2, image, res1 == 1 ? red : white);
	line(0, b3, 1000, a3 * 1000 + b3, image, res2 == 1 ? red : white);*/
}

void drawFile(string fileName, TGAImage &image, TGAColor color) {

	ifstream fichier(fileName, ios::in);  // on ouvre le fichier en lecture

	if (fichier)  // si l'ouverture a réussi
	{
		string option;
		string ligne;
		string trash;
		float intensity, xf, yf , zf;
		int p1, p2, p3;
		int x, y, z;
		vector<Point3D> points = vector<Point3D>();
		vector<Point3D> triangles = vector<Point3D>();
		D2D1_VECTOR_3F lumiere;
		lumiere.x = 0.0;
		lumiere.y = 0.0;
		lumiere.z = 1.0;

		while (getline(fichier, ligne)) {
			if (ligne[0] == 'v' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> xf >> yf >> zf;
				x = xf * (taille / 2) + taille / 2;
				y = yf * (taille / 2) + taille / 2;
				z = zf * (taille / 2) + taille / 2;
				points.push_back(Point3D(x, y, z, xf, yf, zf));
			}
			else if (ligne[0] == 'f' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> p1 >> trash >> p2 >> trash >> p3;
				triangles.push_back(Point3D(p1, p2, p3));
			}
		}

		D2D1_VECTOR_3F n;
		Point3D point1, point2, point3;
		float norme;

		for (int i = 0; i < triangles.size(); i++) {
			point1 = points.at(triangles.at(i).x - 1);
			point2 = points.at(triangles.at(i).y - 1);
			point3 = points.at(triangles.at(i).z - 1);

			n = getNormal(point1, point2, point3);
			norme = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
			n.x /= norme;
			n.y /= norme;
			n.z /= norme;

			intensity = produitScalaire(lumiere, n);

			if (intensity > 0) {
				drawFillTriangle(point1, point2, point3,
					image, calcColor(taille, points.at(triangles.at(i).x - 1).y % taille,intensity));

				/*drawFillTriangle(point1, point2, point3,
					image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));*/
			}
		}

		fichier.close();  // on ferme le fichier
	}
	else {
		cout << "echec ouverture fichier" << endl;
	}
}

#pragma endregion

int main(int argc, char** argv) {
    TGAImage image(taille, taille, TGAImage::RGB);
	//drawFile("diablo3_pose.txt", image, white);
	//drawFile("black_head.txt", image, white);
	drawFile("bogie_head.txt", image, white);
	drawFile("boggie_body.txt", image, white);
	//drawFillTriangle(Point3D(10, 10, 10), Point3D(100, 20, 0), Point3D(90, 90, 0), image, white);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}

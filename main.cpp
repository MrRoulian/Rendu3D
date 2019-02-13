#include "tgaimage.h"
#include <iostream>
#include <string>
#include <sstream> 
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <vector>
#include <time.h>

#include <cmath>
#include <cassert>


using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor gray = TGAColor(125, 125, 125, 255);
const int taille = 1500;
bool anaglyphe = false;
int *zbuffer = new int[taille*taille];


#pragma region Class

struct Matrix {
	vector<vector<float>> data;
	int ligne, colonne;

	Matrix() : ligne(4), colonne(4) { 
		data.resize(ligne);
		for (int i = 0; i < ligne; i++) {
			data[i].resize(colonne);
			for (int j = 0; j < colonne; j++) {
				data[i][j] = 0;
			}
		}
	}; 
	
	Matrix(int ligne, int colonne) : ligne(ligne), colonne(colonne) {
		data.resize(ligne);
		for (int i = 0; i < ligne; i++) {
			data[i].resize(colonne);
			for (int j = 0; j < colonne; j++) {
				data[i][j] = 0;
			}
		}
	};

	Matrix(float f1, float f2, float f3) : ligne(4), colonne(1) {
		data.resize(ligne);
		for (int i = 0; i < ligne; i++) {
			data[i].resize(colonne);
			for (int j = 0; j < colonne; j++) {
				data[i][j] = 0;
			}
		}
		data[0][0] = f1;
		data[1][0] = f2;
		data[2][0] = f3;
		data[3][0] = 1;
	}

	//Retourne une matrice d'identité de taille size
	Matrix (int size) : ligne(size), colonne(size){
		data.resize(ligne);
		for (int i = 0; i < size; i++) {
			data[i].resize(colonne);
			data[i][i] = 1;
		}
	}

	Matrix multiply(Matrix m) {
		assert(colonne == m.ligne);
		Matrix result(ligne, m.colonne);
		for (int i = 0; i < ligne; i++) {
			for (int j = 0; j < m.colonne; j++) {
				result.data[i][j] = 0.f;
				for (int k = 0; k < colonne; k++) {
					result.data[i][j] += data[i][k] * m.data[k][j];
				}
			}
		}
		return result;
	}
};

struct Vec3F {
	float x, y, z;

	Vec3F(float x, float y, float z) : x(x), y(y), z(z) {};

	Vec3F(Matrix m) : x(ceil(m.data[0][0] / m.data[3][0])), y(ceil(m.data[1][0] / m.data[3][0])), z(ceil(m.data[2][0] / m.data[3][0])) {}

	Vec3F() { x = 0; y = 0; z = 0; };

	void normalize() {
		float norme = sqrt(x*x + y*y + z*z);
		x /= norme;
		y /= norme;
		z /= norme;
	}

	float getNorme() {
		return sqrt(x*x + y * y + z * z);
	}

	Vec3F soustraction(Vec3F v) {
		return Vec3F(x - v.x, y - v.y, z - v.z);
	}
};

struct Vec2I {
	int x, y;

	Vec2I(int x, int y) : x(x), y(y) {};

	Vec2I() { x = 0; y = 0; };
};

struct Point3D {
	int x, y, z;
	float xf, yf, zf;

	Point3D(int x, int y, int z) : x(x), y(y), z(z) {};

	Point3D(int x, int y, int z, float xf, float yf, float zf) : x(x), y(y), z(z), xf(xf), yf(yf), zf(zf) {};

	Point3D() { x = 0; y = 0; z = 0; };

	float distance(Point3D p) {
		return sqrt(pow(x - p.x, 2) + (pow(y - p.y, 2) + (pow(z - p.z, 2))));
	}
};

struct Image {
	TGAImage texture = TGAImage();
	TGAImage textureNormal = TGAImage();
	int textH, textW, textNH, textNW;

	Image(const char* textureFile, const char* textureNormalFile) {
		this->texture = TGAImage();
		this->textureNormal = TGAImage();
		this->texture.read_tga_file(textureFile);
		this->textureNormal.read_tga_file(textureNormalFile);
		this->textH = texture.get_height();
		this->textW = texture.get_width();
		this->textNH = textureNormal.get_height();
		this->textNW = textureNormal.get_width();

		texture.flip_vertically();
		textureNormal.flip_vertically();
	};

	Image() {};
};

#pragma endregion

Image img;
Vec3F lumiere;
Vec3F eye;
Vec3F center(0, 0, 0);
Matrix modelView;
Matrix projection;
Matrix viewPort;
Matrix produitMat;

#pragma region Math

Vec3F produitVectoriel (Vec3F vec1, Vec3F vec2) {
	Vec3F res;

	res.x = vec1.y*vec2.z - vec1.z*vec2.y;
	res.y = vec1.z*vec2.x - vec1.x*vec2.z;
	res.z = vec1.x*vec2.y - vec1.y*vec2.x;

	return res;
}

Vec3F getNormal(Point3D vec1, Point3D vec2, Point3D vec3) {
	Vec3F res;

	res.x = (vec2.yf - vec1.yf)*(vec3.zf - vec1.zf) - (vec2.zf - vec1.zf)*(vec3.yf - vec1.yf);
	res.y = (vec2.zf - vec1.zf)*(vec3.xf - vec1.xf) - (vec2.xf - vec1.xf)*(vec3.zf - vec1.zf);
	res.z = (vec2.xf - vec1.xf)*(vec3.yf - vec1.yf) - (vec2.yf - vec1.yf)*(vec3.xf - vec1.xf);

	return res;
}

float produitScalaire(Vec3F vec1, Vec3F vec2) {
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

Vec3F barycentre(Point3D a, Point3D b, Point3D c, Point3D p) {
	Vec3F sous[2];
	sous[0].x = c.x - a.x;
	sous[0].y = b.x - a.x;
	sous[0].z = a.x - p.x;

	sous[1].x = c.y - a.y;
	sous[1].y = b.y - a.y;
	sous[1].z = a.y - p.y;

	Vec3F prodVec = produitVectoriel(sous[0], sous[1]);

	if (abs(prodVec.z) > 1e-2) {
		return Vec3F(1.f - (prodVec.x + prodVec.y) / prodVec.z, prodVec.y / prodVec.z, prodVec.x / prodVec.z);
	}
	else {
		return Vec3F(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
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

Matrix viewport(int x, int y, int w, int h) {
	Matrix mat(4);
	mat.data[0][3] = x + w / 2.f;
	mat.data[1][3] = y + h / 2.f;
	mat.data[2][3] = taille / 2.f;

	mat.data[0][0] = w / 2.f;
	mat.data[1][1] = h / 2.f;
	mat.data[2][2] = taille / 2.f;
	return mat;
}

Matrix lookat(Vec3F eye, Vec3F center, Vec3F up) {
	Vec3F z = (eye.soustraction(center));
	z.normalize();

	Vec3F x = produitVectoriel(up,z);
	x.normalize();

	Vec3F y = produitVectoriel(z,x);
	y.normalize();

	Matrix res(4);

	res.data[0][0] = x.x;
	res.data[1][0] = y.x;
	res.data[2][0] = z.x;
	res.data[0][3] = -center.x;

	res.data[0][1] = x.y;
	res.data[1][1] = y.y;
	res.data[2][1] = z.y;
	res.data[1][3] = -center.y;

	res.data[0][2] = x.z;
	res.data[1][2] = y.z;
	res.data[2][2] = z.z;
	res.data[2][3] = -center.z;

	return res;
}

void changerFond(TGAImage &img) {
	int h = img.get_height();
	int w = img.get_width();
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			img.set(i, j, gray);
		}
	}
}

void viderZbuffer() {
	for (int i = 0; i < taille*taille; i++) {
		zbuffer[i] = -1;
	}
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

void drawFillTriangle(Point3D p1, Point3D p2, Point3D p3, TGAImage &image, Vec3F v1, Vec3F v2, Vec3F v3, int numImage) {
	//Variable qui servent pour l'opti
	vector<int> tabX = vector<int>(); tabX.push_back(p1.x); tabX.push_back(p2.x); tabX.push_back(p3.x);
	vector<int> tabY = vector<int>(); tabY.push_back(p1.y); tabY.push_back(p2.y); tabY.push_back(p3.y);

	Point3D courant;
	TGAColor color;
	TGAColor colorNormal;
	Vec3F texturePoint;
	Vec3F n;
	float intensity;

	int xMin = minTab(tabX);
	int xMax = maxTab(tabX);
	int yMin = minTab(tabY);
	int yMax = maxTab(tabY);

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
	for (int j = yMin; j < yMax; j++) {

		for (int i = xMin; i < xMax; i++) {

			courant = Point3D(i, j, 0);
			Vec3F vecteurBari = barycentre(p1, p2, p3, courant);

			texturePoint.x = v1.x * vecteurBari.x + v2.x * vecteurBari.y + v3.x * vecteurBari.z;
			texturePoint.y = v1.y * vecteurBari.x + v2.y * vecteurBari.y + v3.y * vecteurBari.z;

			color = img.texture.get(texturePoint.x*img.textW, texturePoint.y*img.textH);
			colorNormal = img.textureNormal.get(texturePoint.x*img.textNW, texturePoint.y*img.textNH);


			//Normalisation
			//n = getNormal(p1, p2, p3).normalize();
			n = Vec3F((float)colorNormal.raw[2] / 128 - 1, (float)colorNormal.raw[1] / 128 - 1, (float)colorNormal.raw[0] / 128 - 1);

			//Calcul de l'intensité
			intensity = max(0.f, min(1.f, produitScalaire(lumiere, n)));


			for (int k = 0; k < 3; k++) color.raw[k] *= intensity;

			courant.z = 0;

			courant.z += p1.z * vecteurBari.x;
			courant.z += p2.z * vecteurBari.y;
			courant.z += p3.z * vecteurBari.z;

			//Si je suis dans le triangle
			if (res * j >= res * (a1 * i + b1) && res1 * j >= res1 * (a2 * i + b2) && res2 * j >= res2 * (a3 * i + b3)) {
				if (zbuffer[i + j * taille] <= courant.z) {
					zbuffer[i + j * taille] = courant.z;

					if (anaglyphe) {
						if (numImage %2 == 0) {
							color.raw[0] = image.get(i, j).raw[0];
							color.raw[1] = image.get(i, j).raw[1];
						} else {
							color.raw[2] = image.get(i, j).raw[2];
						}

						image.set(i, j, color);
					}
					else {
						image.set(i + taille * numImage, j, color);
					}
				}
			}
		}
	}
}

void drawFile(string fileName, TGAImage &image, int numImage) {

	ifstream fichier(fileName, ios::in);  // on ouvre le fichier en lecture

	if (fichier)  // si l'ouverture a réussi
	{
		string option;
		string ligne;
		string trash;
		float xf, yf , zf;
		int p1, p2, p3;
		int t1, t2, t3;
		int x, y, z, poubelle;
		char slash;
		vector<Point3D> points = vector<Point3D>();
		vector<Point3D> triangles = vector<Point3D>();
		vector<Point3D> centerF = vector<Point3D>();
		vector<Vec3F> textCoord = vector<Vec3F>();
		Vec3F v;

		while (getline(fichier, ligne)) {
			if (ligne[0] == 'v' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> xf >> yf >> zf;

				v = Vec3F(produitMat.multiply(Matrix(xf,yf,zf)));

				v.x = max(min(taille + 0.f, v.x),0.f);

				v.y = max(min(taille + 0.f, v.y),0.f);

				points.push_back(Point3D(v.x, v.y, v.z, xf, yf, zf));
			}
			else if (ligne[0] == 'v' && ligne[1] == 't') {
				istringstream(ligne) >> option >> xf >> yf;
				textCoord.push_back(Vec3F(xf, yf, 0));
			}
			else if (ligne[0] == 'f' && ligne[1] == ' ') {
				istringstream(ligne) >> option >> p1 >> slash >> t1 >> trash >> p2 >> slash >> t2 >> trash >> p3 >> slash >> t3 >> trash;
				centerF.push_back(Point3D(abs(t1), abs(t2), abs(t3)));
				triangles.push_back(Point3D(abs(p1), abs(p2), abs(p3)));
			}
		}


		Point3D point1, point2, point3;
		Vec3F v1, v2, v3;

		for (int i = 0; i < triangles.size(); i++) {

			//Récup des couleurs des trois points
			v1 = textCoord.at(centerF.at(i).x-1);
			v2 = textCoord.at(centerF.at(i).y-1);
			v3 = textCoord.at(centerF.at(i).z-1);

			//Points du triangle
			point1 = points.at(triangles.at(i).x - 1);
			point2 = points.at(triangles.at(i).y - 1);
			point3 = points.at(triangles.at(i).z - 1);
			
			drawFillTriangle(point1, point2, point3, image, v1, v2, v3, numImage);
		}

		fichier.close();  // on ferme le fichier
	}
	else {
		cout << "echec ouverture fichier" << endl;
	}

	if (anaglyphe && numImage == 0) {
		eye.x += 0.2;

		modelView = lookat(eye, center, Vec3F(0, 1, 0));
		projection = Matrix(4);
		viewPort = viewport(taille / 8, taille / 8, taille * 3 / 4, taille * 3 / 4);
		projection.data[3][2] = -1.f / (eye.soustraction(center)).getNorme();

		produitMat = viewPort.multiply(projection);
		produitMat = produitMat.multiply(modelView);

		for (int i = 0; i < taille*taille; i++) {
			zbuffer[i] = -1;
		}

		drawFile(fileName, image, 1);

		eye.x -= 0.2;

		modelView = lookat(eye, center, Vec3F(0, 1, 0));
		projection = Matrix(4);
		viewPort = viewport(taille / 8, taille / 8, taille * 3 / 4, taille * 3 / 4);
		projection.data[3][2] = -1.f / (eye.soustraction(center)).getNorme();

		produitMat = viewPort.multiply(projection);
		produitMat = produitMat.multiply(modelView);
	}
}

#pragma endregion

int main(int argc, char** argv) {

	eye = Vec3F(1, 1, 3);
	lumiere = eye;
	lumiere.normalize();

	bool diablo = true;
	bool affrican = false;

	int nbImages = 1;

	string textureFile;
	string nmFile;
	string obj;

	TGAImage image = TGAImage(taille*nbImages, taille, TGAImage::RGB);
	changerFond(image);

	#pragma region  calculProjec
	modelView = lookat(eye, center, Vec3F(0, 1, 0));
	projection = Matrix(4);
	viewPort = viewport(taille / 8, taille / 8, taille * 3 / 4, taille * 3 / 4);
	projection.data[3][2] = -1.f / (eye.soustraction(center)).getNorme();

	produitMat = viewPort.multiply(projection);
	produitMat = produitMat.multiply(modelView);
	#pragma endregion

	if (diablo) {
		textureFile = "obj/diablo3_pose/diablo3_pose_diffuse.tga";
		nmFile = "obj/diablo3_pose/diablo3_pose_nm.tga";
		obj = "obj/diablo3_pose/diablo3_pose.obj";

		img = Image(textureFile.c_str(), nmFile.c_str());
		drawFile(obj, image, 0);
	}
	
	if (affrican) {
		textureFile = "obj/african_head/african_head_diffuse.tga";
		nmFile = "obj/african_head/african_head_nm.tga";
		obj = "obj/african_head/african_head.obj";
		
		img = Image(textureFile.c_str(), nmFile.c_str());
		drawFile(obj, image, 0);

		textureFile = "obj/african_head/african_head_eye_inner_diffuse.tga";
		nmFile = "obj/african_head/african_head_eye_inner_nm.tga";
		obj = "obj/african_head/african_head_eye_inner.obj";

		img = Image(textureFile.c_str(), nmFile.c_str());
		drawFile(obj, image, 0);
	}

	/*
	"obj/african_head/african_head_eye_inner_diffuse.tga"
	"obj/african_head/african_head_eye_inner_nm.tga"	
	"obj/african_head/african_head_eye_inner.obj"

	img = Image("obj/maya/SirenBody_Bm.tga","obj/maya/SirenBody_Nrm.tga");
	drawFile("obj/maya/maya.obj",image,0);
	
	img = Image("obj/boggie/body_diffuse.tga", "obj/boggie/body_nm_tangent.tga");
	drawFile("obj/boggie/body.obj", image,2);

	img = Image("obj/boggie/head_diffuse.tga", "obj/boggie/head_nm_tangent.tga");
	drawFile("obj/boggie/head.obj", image,2);

	img = Image("obj/boggie/eyes_diffuse.tga", "obj/boggie/eyes_nm_tangent.tga");
	drawFile("obj/boggie/eyes.obj", image,2);
	*/

	
	/*for (int k = 0; k < nbImages; k++) {
		//eye = eyes[k];
		lumiere = eye;
		lumiere.normalize();

		modelView = lookat(eye, center, Vec3F(0, 1, 0));
		projection = Matrix(4);
		viewPort = viewport(taille / 8, taille / 8, taille * 3 / 4, taille * 3 / 4);
		projection.data[3][2] = -1.f / (eye.soustraction(center)).getNorme();

		produitMat = viewPort.multiply(projection);
		produitMat = produitMat.multiply(modelView);

		viderZbuffer();

		cout << k << endl;

		drawFile(obj, image, k);

	}*/

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}

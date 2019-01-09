#include "tgaimage.h"
#include "Point.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>


using namespace std;


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

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

void triangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	line(x0, y0, x1, y1, image, color);
	line(x1, y1, x2, y2, image, color);
	line(x2, y2, x0, y0, image, color);
}

void drawPoint(int x0, int y0, TGAImage &image, TGAColor color) {
	image.set(x0, y0, color);
}

void drawFile(string fileName, TGAImage &image, TGAColor colorcolor, int taille) {

	ifstream fichier(fileName, ios::in);  // on ouvre le fichier en lecture
	


	if (fichier)  // si l'ouverture a réussi
	{
		string option;
		float x, y, z;
		int p1, p2, p3;
		int indice = 0;
		char trash;
		int trashInt;
		Point *tab = new Point[6000];

		
		fichier >> option >> x >> y >> z;

		do {

			x = x * (taille / 2) + taille / 2;
			y = y * (taille / 2) + taille / 2;
			Point p = Point(x, y, z);
			tab[indice] = p;
			indice++;
			drawPoint(x, y, image, white);
			fichier >> option >> x >> y >> z;


		} while (option == "v");

		/*while (option != "f") { 
			getline(fichier, option);
			cout << option << endl;
		};*/

		/*do {
			fichier >> option >> p1 >> trash >> trashInt >> trash >> trashInt >> trash >> p2 >> trash >> trashInt >> trash >> trashInt >> trash >> p3 >> trash >> trashInt >> trash >> trashInt >> trash;
			triangle(tab[p1].x, tab[p1].y, tab[p2].x, tab[p2].y, tab[p3].x, tab[p3].y, image, color);
		} while (option == "f");*/



		fichier.close();  // on ferme le fichier
	}
	else {
		cout << "echec ouverture fichier" << endl;
	}
}

int main(int argc, char** argv) {
    TGAImage image(1000, 1000, TGAImage::RGB);
	drawFile("diablo3_pose.txt", image, white, 1000);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}

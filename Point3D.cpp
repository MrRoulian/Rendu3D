#include "Point3D.h"



Point3D::Point3D(int x, int y, int z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Point3D::Point3D(int x, int y, int z, float xf, float yf, float zf)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->xf = xf;
	this->yf = yf;
	this->zf = zf;
}

Point3D::Point3D() {

}

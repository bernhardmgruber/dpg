#include <cmath>
#include <iomanip>

#include "mathlib.h"

using namespace std;

ostream& operator<<(ostream& os, Vector3D v)
{
    os << "(" << setw(2) << v.x << ", " << v.y << ", " << v.z << ")";

    return os;
}

ostream& operator<<(ostream& os, Vector2D v)
{
    os << "(" << setw(2) << v.x << ", " << v.y << ")";

    return os;
}

Vector3D operator-(Vector3D v1, Vector3D v2)
{
    Vector3D resultVector;
    resultVector.x = v1.x - v2.x;
    resultVector.y = v1.y - v2.y;
    resultVector.z = v1.z - v2.z;
    return resultVector;
}

Vector3D operator+(Vector3D v1, Vector3D v2)
{
    Vector3D resultVector;
    resultVector.x = v1.x + v2.x;
    resultVector.y = v1.y + v2.y;
    resultVector.z = v1.z + v2.z;
    return resultVector;
}

Vector3D operator*(float f, Vector3D v)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    return v;
}

Vector3D operator*(Vector3D v, float f)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    return v;
}

Vector3D operator/(Vector3D v, float f)
{
    v.x /= f;
    v.y /= f;
    v.z /= f;
    return v;
}

bool operator==(Vector3D v1, Vector3D v2)
{
	Vector3D v = v1 - v2;

	/*if(fabs(v.x) > EPSILON)
		return false;
	if(fabs(v.y) > EPSILON)
		return false;
	if(fabs(v.z) > EPSILON)
		return false;*/

    if(v.x != 0)
        return false;
    if(v.y != 0)
        return false;
    if(v.z != 0)
        return false;

	return true;
}

bool operator!=(Vector3D v1, Vector3D v2)
{
	return !(v1 == v2);
}

float Length(Vector3D v)
{
    return sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}

Vector3D Normalize(Vector3D v)
{
    return v / Length(v);
}

float DotProduct(Vector3D v1, Vector3D v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

Vector3D CrossProduct(Vector3D v1, Vector3D v2)
{
    Vector3D perpendicularVector;
    perpendicularVector.x = (v2.y * v1.z) - (v2.z * v1.y);
    perpendicularVector.y = (v2.z * v1.x) - (v2.x * v1.z);
    perpendicularVector.z = (v2.x * v1.y) - (v2.y * v1.x);
    return perpendicularVector;
}

bool PointInBox(Vector3D vPoint, short vMin[3], short vMax[3])
{
    if(((float)vMin[0] <= vPoint.x && vPoint.x <= (float)vMax[0] &&
        (float)vMin[1] <= vPoint.y && vPoint.y <= (float)vMax[1] &&
        (float)vMin[2] <= vPoint.z && vPoint.z <= (float)vMax[2]) ||
       ((float)vMin[0] >= vPoint.x && vPoint.x >= (float)vMax[0] &&
        (float)vMin[1] >= vPoint.y && vPoint.y >= (float)vMax[1] &&
        (float)vMin[2] >= vPoint.z && vPoint.z >= (float)vMax[2]))
        return true;
    else
        return false;
}

bool PointInPlane(Vector3D vPoint, Vector3D vNormal, float fDist)
{
    if(fabs(DotProduct(vPoint, vNormal) - fDist) < EPSILON)
        return true;
    else
        return false;
}

Vector3D RotateX(float a, Vector3D v)
{
    a = DEGTORAD(a);

    Vector3D res;
    res.x = v.x;
    res.y = v.y * cos(a) + v.z * -sin(a);
    res.z = v.y * sin(a) + v.z *  cos(a);
    return res;
}

Vector3D RotateY(float a, Vector3D v)
{
    a = DEGTORAD(a);

    Vector3D res;
    res.x = v.x *  cos(a) + v.z * sin(a);
    res.y = v.y;
    res.z = v.x * -sin(a) + v.z * cos(a);
    return res;
}

Vector3D RotateZ(float a, Vector3D v)
{
    a = DEGTORAD(a);

    Vector3D res;
    res.x = v.x * cos(a) + v.y * -sin(a);
    res.y = v.x * sin(a) + v.y *  cos(a);
    res.z = v.z;
    return res;
}

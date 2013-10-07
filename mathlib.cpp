#include <cmath>
#include <iomanip>

#include "mathlib.h"

using namespace std;

ostream& operator<<(ostream& os, Vector3F v)
{
    os << "(" << setw(2) << v.x << ", " << v.y << ", " << v.z << ")";

    return os;
}

ostream& operator<<(ostream& os, Vector3I v)
{
    os << "(" << setw(2) << v.x << ", " << v.y << ", " << v.z << ")";

    return os;
}

ostream& operator<<(ostream& os, Vector2F v)
{
    os << "(" << setw(2) << v.x << ", " << v.y << ")";

    return os;
}

Vector3F operator-(Vector3F v1, Vector3F v2)
{
    Vector3F resultVector;
    resultVector.x = v1.x - v2.x;
    resultVector.y = v1.y - v2.y;
    resultVector.z = v1.z - v2.z;
    return resultVector;
}

Vector3F operator+(Vector3F v1, Vector3F v2)
{
    Vector3F resultVector;
    resultVector.x = v1.x + v2.x;
    resultVector.y = v1.y + v2.y;
    resultVector.z = v1.z + v2.z;
    return resultVector;
}

Vector3I operator+(Vector3I v1, Vector3I v2)
{
    Vector3I resultVector;
    resultVector.x = v1.x + v2.x;
    resultVector.y = v1.y + v2.y;
    resultVector.z = v1.z + v2.z;
    return resultVector;
}

Vector3F operator*(float f, Vector3F v)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    return v;
}

Vector3F operator*(Vector3F v, float f)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    return v;
}

Vector3F operator/(Vector3F v, float f)
{
    v.x /= f;
    v.y /= f;
    v.z /= f;
    return v;
}

bool operator==(Vector3F v1, Vector3F v2)
{
	Vector3F v = v1 - v2;

    if(v.x != 0)
        return false;
    if(v.y != 0)
        return false;
    if(v.z != 0)
        return false;

	return true;
}

bool operator!=(Vector3F v1, Vector3F v2)
{
	return !(v1 == v2);
}

float length(Vector3F v)
{
    return sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}

Vector3F normalize(Vector3F v)
{
    return v / length(v);
}

float dotProduct(Vector3F v1, Vector3F v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

Vector3F crossProduct(Vector3F v1, Vector3F v2)
{
    Vector3F perpendicularVector;
    perpendicularVector.x = (v2.y * v1.z) - (v2.z * v1.y);
    perpendicularVector.y = (v2.z * v1.x) - (v2.x * v1.z);
    perpendicularVector.z = (v2.x * v1.y) - (v2.y * v1.x);
    return perpendicularVector;
}

bool pointInBox(Vector3F vPoint, short vMin[3], short vMax[3])
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

bool pointInPlane(Vector3F vPoint, Vector3F vNormal, float fDist)
{
    if(fabs(dotProduct(vPoint, vNormal) - fDist) < EPSILON)
        return true;
    else
        return false;
}

Vector3F rotateX(float a, Vector3F v)
{
    a = DEGTORAD(a);

    Vector3F res;
    res.x = v.x;
    res.y = v.y * cos(a) + v.z * -sin(a);
    res.z = v.y * sin(a) + v.z *  cos(a);
    return res;
}

Vector3F rotateY(float a, Vector3F v)
{
    a = DEGTORAD(a);

    Vector3F res;
    res.x = v.x *  cos(a) + v.z * sin(a);
    res.y = v.y;
    res.z = v.x * -sin(a) + v.z * cos(a);
    return res;
}

Vector3F rotateZ(float a, Vector3F v)
{
    a = DEGTORAD(a);

    Vector3F res;
    res.x = v.x * cos(a) + v.y * -sin(a);
    res.y = v.x * sin(a) + v.y *  cos(a);
    res.z = v.z;
    return res;
}

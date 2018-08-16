#ifndef RAY_H
#define RAY_H
#include "Vector3.h"

class Ray
{
    public:
        Ray() {}
        Ray(const Vector3& a, const Vector3& b) { A = a; B = b; }  
        Vector3 origin() const       { return A; }
        Vector3 direction() const    { return B; }
        Vector3 point_at_parameter(float t) const { return A + t*B; }

        Vector3 A;
        Vector3 B;
};

#endif 
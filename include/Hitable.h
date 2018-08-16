#ifndef HITABLE_H
#define HITABLE_H 

#include "Ray.h"

class Material;

struct hit_record
{
    float t;  
    Vector3 p;
    Vector3 normal; 
    Material *mat_ptr;
};

class Hitable  {
    public:
        virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};

#endif
#ifndef MATERIAL_H
#define MATERIAL_H

struct hit_record;

#include "Random.h"
#include "Ray.h"
#include "Hitable.h"

float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

bool refract(const Vector3& v, const Vector3& n, float ni_over_nt, Vector3& refracted) {
    Vector3 uv = unit_vector(v);
    Vector3 un = unit_vector(n);

    float dt = dot(uv, un);
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(uv - dt*un) - sqrt(discriminant)*un;
        return true;
    }
    else
        return false;
}

Vector3 reflect(const Vector3& v, const Vector3& n) {
     return v - 2*dot(v,n)*n;
}

Vector3 random_in_unit_sphere() {
    Vector3 p;
    do {
        p = 2.0*Vector3(drand48(),drand48(),drand48()) - Vector3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}

class Material  {
    public:
        virtual bool scatter(const Ray& r_in, const hit_record& rec, Vector3& attenuation, Ray& scattered) const = 0;
};

class lambertian : public Material {
    public:
        lambertian(const Vector3& a) : albedo(a) {}
        virtual bool scatter(const Ray& r_in, const hit_record& rec, Vector3& attenuation, Ray& scattered) const  {
             Vector3 target = rec.p + rec.normal + random_in_unit_sphere();
             scattered = Ray(rec.p, target-rec.p);
             attenuation = albedo;
             return true;
        }

        Vector3 albedo;
};

class metal : public Material {
    public:
        metal(const Vector3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
        virtual bool scatter(const Ray& r_in, const hit_record& rec, Vector3& attenuation, Ray& scattered) const  {
            Vector3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = Ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        Vector3 albedo;
        float fuzz;
};

class dielectric : public Material {
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const Ray& r_in, const hit_record& rec, Vector3& attenuation, Ray& scattered) const  {
             Vector3 outward_normal;
             Vector3 reflected = reflect(r_in.direction(), rec.normal);
             float ni_over_nt;
             attenuation = Vector3(1.0, 1.0, 1.0);
             Vector3 refracted;
             float reflect_prob;
             float cosine;
             if (dot(r_in.direction(), rec.normal) > 0) {
                  outward_normal = -rec.normal;
                  ni_over_nt = ref_idx;
                    //cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
             }
             else {
                  outward_normal = rec.normal;
                  ni_over_nt = 1.0f / ref_idx;
                  cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
             }
             if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
                reflect_prob = schlick(cosine, ref_idx);
             else
                reflect_prob = 1.0;
             if (drand48() < reflect_prob)
                scattered = Ray(rec.p, reflected);
             else
                scattered = Ray(rec.p, refracted);
             return true;
        }

        float ref_idx;
};

#endif
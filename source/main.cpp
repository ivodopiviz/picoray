#include <iostream>
#include <fstream>
#include <float.h>

#define _USE_MATH_DEFINES // TODO: get rid of this
#include <math.h>

#include "Random.h"
#include "Sphere.h"
#include "HitableList.h"
#include "Camera.h"
#include "Material.h"

Vector3 color(const Ray& r, Hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX , rec)) { 
        Ray scattered;
        Vector3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {
            return Vector3(0,0,0);
        }
    }
    else {
        Vector3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*Vector3(1.0, 1.0, 1.0) + t*Vector3(0.5, 0.7, 1.0);
    }
}

Hitable *random_scene() {
    int n = 500;
    Hitable **list = new Hitable*[n+1];
    list[0] =  new Sphere(Vector3(0,-1000,0), 1000, new lambertian(Vector3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            Vector3 center(a+0.9*drand48(),0.2,b+0.9*drand48()); 
            if ((center-Vector3(4,0.2,0)).length() > 0.9) { 
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new Sphere(center, 0.2, new lambertian(Vector3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new Sphere(center, 0.2,
                            new metal(Vector3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())),  0.5*drand48()));
                }
                else {  // glass
                    list[i++] = new Sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new Sphere(Vector3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new Sphere(Vector3(-4, 1, 0), 1.0, new lambertian(Vector3(0.4, 0.2, 0.1)));
    list[i++] = new Sphere(Vector3(4, 1, 0), 1.0, new metal(Vector3(0.7, 0.6, 0.5), 0.0));

    return new HitableList(list,i);
}

int main() {
    int nx = 1280;
    int ny = 720;

	//int nx = 200;
	//int ny = 100;

    int ns = 10;

	std::ofstream test_file;
	test_file.open("test.ppm");

	test_file << "P3\n" << nx << " " << ny << "\n255\n";
    Hitable *list[5];
    float R = cos(M_PI/4);
    list[0] = new Sphere(Vector3(0,0,-1), 0.5, new lambertian(Vector3(0.1, 0.2, 0.5)));
    list[1] = new Sphere(Vector3(0,-100.5,-1), 100, new lambertian(Vector3(0.8, 0.8, 0.0)));
    list[2] = new Sphere(Vector3(1,0,-1), 0.5, new metal(Vector3(0.8, 0.6, 0.2), 0.0));
    list[3] = new Sphere(Vector3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new Sphere(Vector3(-1,0,-1), -0.45, new dielectric(1.5));
    Hitable *world = new HitableList(list,5);
    world = random_scene();

    Vector3 lookfrom(13,2,3);
    Vector3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    Camera cam(lookfrom, lookat, Vector3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

	std::cout << "Started Rendering." << std::endl;
    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            Vector3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                Ray r = cam.getRay(u, v);
                Vector3 p = r.point_at_parameter(2.0);
                col += color(r, world,0);
            }
            col /= float(ns);
            col = Vector3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]); 
            int ig = int(255.99*col[1]); 
            int ib = int(255.99*col[2]); 
			test_file << ir << " " << ig << " " << ib << "\n";
        }

		std::cout << "Finished Rendering Scanline: " << ny - j << "/"  << ny << std::endl;
    }

	test_file.close();
	std::cout << "Finished Rendering." << std::endl;
}
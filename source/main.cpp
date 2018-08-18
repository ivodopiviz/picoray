#include <iostream>
#include <fstream>
#include <float.h>
#include <vector>
#include <SDL.h>

#include "Random.h"
#include "Sphere.h"
#include "HitableList.h"
#include "Camera.h"
#include "Material.h"

SDL_sem* gDataLock = nullptr;
bool gTracing = true;

Vector3 color(const Ray& r, Hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001f, FLT_MAX , rec)) { 
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
                            new metal(Vector3(0.5f*(1.f + drand48()), 0.5f*(1.f + drand48()), 0.5f*(1.f + drand48())),  0.5f*drand48()));
                }
                else {  // glass
                    list[i++] = new Sphere(center, 0.2f, new dielectric(1.5f));
                }
            }
        }
    }

    list[i++] = new Sphere(Vector3(0.f, 1.f, 0.f), 1.0f, new dielectric(1.5f));
    list[i++] = new Sphere(Vector3(-4.f, 1.f, 0.f), 1.0f, new lambertian(Vector3(0.4f, 0.2f, 0.1f)));
    list[i++] = new Sphere(Vector3(4.f, 1.f, 0.f), 1.0f, new metal(Vector3(0.7f, 0.6f, 0.5f), 0.0f));

    return new HitableList(list,i);
}

struct worker_data {
	int full_width;
	int full_height;
	int region_width;
	int region_height;

	Camera* camera;
	Hitable* world;

	Uint8* pixels;
};

int worker_function(void *pData) {
	worker_data* data = (worker_data*)pData;

	bool tracing = true;

	int j = data->region_height - 1;
	int i = 0;
	int ns = 10;

	while (gTracing) {
		Vector3 col(0, 0, 0);
		for (int s = 0; s < ns; s++) {
			float u = float(i + drand48()) / float(data->full_width);
			float v = float(j + drand48()) / float(data->full_height);
			Ray r = data->camera->getRay(u, v);
			Vector3 p = r.point_at_parameter(2.0);
			col += color(r, data->world, 0);
		}
		col /= float(ns);
		col = Vector3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
		Uint8 ir = int(255.99*col[0]);
		Uint8 ig = int(255.99*col[1]);
		Uint8 ib = int(255.99*col[2]);

		std::cout << "Worker Locking" << std::endl;
		SDL_SemWait(gDataLock);
		const unsigned int offset = (data->region_width * 4 * (data->region_height - j)) + i * 4;
		data->pixels[offset + 0] = SDL_ALPHA_OPAQUE;  // a
		data->pixels[offset + 1] = ir;				// r
		data->pixels[offset + 2] = ig;				// g
		data->pixels[offset + 3] = ib;				// b
		SDL_SemPost(gDataLock);
		std::cout << "Worker Unlocking" << std::endl;

		i++;

		if (i > data->region_width - 1)
		{
			j--;
			i = 0;
		}

		if (j <= 0)
		{
			gTracing = false;
			std::cout << "Tracing is finished" << std::endl;
		}

		SDL_Delay(10);
	}

	std::cout << "Thread Finished!" << std::endl;
	return 0;
}

int main(int argc, char* args[]) {
	SDL_Thread* worker = nullptr;

    int nx = 640;
    int ny = 360;

	int rx = nx / 2;
	int ry = ny / 2;

    int ns = 10;

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_Window* window;
	SDL_Renderer* renderer;

	if (SDL_CreateWindowAndRenderer(nx, ny, 0, &window, &renderer) == -1) {
		std::cout << " Failed to initialize Window and Renderer : " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_SetWindowTitle(window, "picoray");

	gDataLock = SDL_CreateSemaphore(1);

	Hitable* list[5];
	float R = cos(M_PI/4);
	list[0] = new Sphere(Vector3(0,0,-1), 0.5, new lambertian(Vector3(0.1, 0.2, 0.5)));
	list[1] = new Sphere(Vector3(0,-100.5,-1), 100, new lambertian(Vector3(0.8, 0.8, 0.0)));
	list[2] = new Sphere(Vector3(1,0,-1), 0.5, new metal(Vector3(0.8, 0.6, 0.2), 0.0));
	list[3] = new Sphere(Vector3(-1,0,-1), 0.5, new dielectric(1.5));
	list[4] = new Sphere(Vector3(-1,0,-1), -0.45, new dielectric(1.5));
	Hitable* world = new HitableList(list,5);
	world = random_scene();

	Vector3 lookfrom(13.f, 2.f, 3.f);
	Vector3 lookat(0.f, 0.f, 0.f);
	float dist_to_focus = 10.0f;
	float aperture = 0.1f;

	Camera cam(lookfrom, lookat, Vector3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

	//Main loop flag 
	bool quit = false; 
	bool tracing = true;

	//Event handler 
	SDL_Event e;

	//int j = ny - 1;
	int j = ry - 1;
	int i = 0;

	std::vector<Uint8> pixels(nx * ny * 4, 0);
	SDL_Texture* buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, nx, ny);

	worker_data data;
	data.full_width = nx;
	data.full_height = ny;
	data.region_width = nx / 2;
	data.region_height = ny / 2;
	data.world = world;
	data.camera = &cam;
	//data.pixels = std::vector<Uint8>(data.region_width * data.region_height * 4);
	data.pixels = new Uint8[data.region_width * data.region_height * 4];
	for (int i = 0; i < data.region_width * data.region_height * 4; i++) {
		data.pixels[i] = 0;    // Initialize all elements to zero.
	}
	//memset(data.pixels, 0, data.region_width * data.region_height * 4);

	worker = SDL_CreateThread(worker_function, "Worker 1", (void*) &data);

	SDL_Rect update_rect;
	update_rect.x = 0;
	update_rect.y = 0;
	update_rect.w = rx;
	update_rect.h = ry;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		//if (tracing) {
		//	// trace a pixel
		//	Vector3 col(0, 0, 0);
		//	for (int s = 0; s < ns; s++) {
		//		float u = float(i + drand48()) / float(nx);
		//		float v = float(j + drand48()) / float(ny);
		//		Ray r = cam.getRay(u, v);
		//		Vector3 p = r.point_at_parameter(2.0);
		//		col += color(r, world, 0);
		//	}
		//	col /= float(ns);
		//	col = Vector3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
		//	Uint8 ir = int(255.99*col[0]);
		//	Uint8 ig = int(255.99*col[1]);
		//	Uint8 ib = int(255.99*col[2]);

		//const unsigned int offset = (rx * 4 * (ry - j)) + i * 4;
		//	pixels[offset + 0] = SDL_ALPHA_OPAQUE;  // a
		//	pixels[offset + 1] = ir;				// r
		//	pixels[offset + 2] = ig;				// g
		//	pixels[offset + 3] = ib;				// b

		//data.pixels[offset + 0] = SDL_ALPHA_OPAQUE;  // a
		//data.pixels[offset + 1] = 255;				// r
		//data.pixels[offset + 2] = 0;				// g
		//data.pixels[offset + 3] = 255;				// b

		std::cout << "Main Locking" << std::endl;
		SDL_SemWait(gDataLock);
		SDL_UpdateTexture(buffer, &update_rect, data.pixels, rx * 4);
		SDL_SemPost(gDataLock);
		std::cout << "Main Unlocking" << std::endl;

		//	i++;

		//	if (i > rx - 1)
		//	{
		//		j--;
		//		i = 0;
		//	}

		//	if (j < 0)
		//	{
		//		tracing = false;
		//		std::cout << "Tracing is finished" << std::endl;
		//	}
		//}

		SDL_RenderCopy(renderer, buffer, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
	gTracing = false;

	SDL_WaitThread(worker, NULL);

	delete data.pixels;
	pixels.clear();

	SDL_DestroySemaphore(gDataLock);
	gDataLock = NULL;

	SDL_DestroyTexture(buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
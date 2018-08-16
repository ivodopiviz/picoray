#include <iostream>
#include <fstream>

int main() {
	int nx = 200;
	int ny = 100;

	std::ofstream test_file;
	test_file.open("test.ppm");

	test_file << "P3\n" << nx << " " << ny << "\n255\n";
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			float r = float(i) / float(nx);
			float g = float(j) / float(ny);
			float b = 0.2;

			float ir = int(255.99*r);
			float ig = int(255.99*g);
			float ib = int(255.99*b);
			test_file << ir << " " << ig << " " << ib << "\n";
		}
	}

	test_file.close();
}
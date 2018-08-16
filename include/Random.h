#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <iostream>
std:: default_random_engine generator;
std::uniform_real_distribution<double> distr(0.0, 1.0);

double drand48(){
    return distr(generator);
}
#endif

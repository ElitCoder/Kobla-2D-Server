#include "Random.h"

#include <random>

using namespace std;

int Random::getRandomInteger(int from, int to) {
	random_device device;
	mt19937 mt(device());
	uniform_int_distribution<int> distribution(from, to);
	
	return distribution(mt);
}

double Random::getRandomFloating(double from, double to) {
	random_device device;
	mt19937 mt(device());
	uniform_real_distribution<double> distribution(from, to);
	
	return distribution(mt);
}
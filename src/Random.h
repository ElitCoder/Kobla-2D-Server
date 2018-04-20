#pragma once
#ifndef RANDOM_H
#define RANDOM_H

class Random {
public:
	static int getRandomInteger(int from, int to);
	static double getRandomFloating(double from, double to);
};

#endif
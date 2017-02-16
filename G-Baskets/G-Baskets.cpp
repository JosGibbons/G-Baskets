// Author Jos Gibbons, February 2017
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <cstddef>
#include <windows.h>

// Custom class for 3d vectors.
class v3d
{
public:
	v3d(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	};
	inline v3d operator +(const v3d &b)
	{
		return v3d
		(
			x + b.x,
			y + b.y,
			z + b.z
		);
	};
	inline v3d operator *(double b)
	{
		return v3d
		(
			x*b,
			y*b,
			z*b
		);
	};
	inline v3d operator -(double b)
	{
		return v3d
		(
			x,
			y,
			z - b
		);
	};
	double x;
	double y;
	double z;
};

//A parabolic path due to downward constant acceleration starts at x = x0 with v = v0 at time 0.
v3d Parabola(v3d x0, v3d v0, double t, double g = 9.81)
{
	v3d gravity = v3d(0, 0, -abs(g));
	return x0 + v0 * t + gravity * (0.5 * t * t);
}

// Parabola reaches ground at t > 0 obtained by the quadratic formula, unless x_{01} < 0. If necessary we return a negative "endtime".
double Endtime(double x0z, double v0z, double g = 9.81)
{
	return (v0z + sqrt(pow(v0z, 2) + 2 * x0z * abs(g))) / abs(g);
}

// Custom class for 2d vectors.
class v2d
{
public:
	v2d(double x, double y)
	{
		this->x = x;
		this->y = y;
	}
	double x;
	double y;
};

// Throw away a 3D vector's z-coordinate.
v2d Flatten(v3d a)
{
	return v2d(a.x, a.y);
}

// Compute the x, y-coordinates of a parabola's z == 0, t > 0 landing site.
v2d LandingSite(v3d x0, v3d v0, double g = 9.81)
{
	double t = Endtime(x0.z, v0.z, g);
	return Flatten(Parabola(x0, v0, t, g));
}

// Computer decides how fast to launch the ball. We compute v_z0 so the parabola's acme is (1 + tolerance) times the target's height.
v3d HintV0(v3d centre, double g = 9.81, double tolerance = 0.1)
{
	double scale = 1 + tolerance;
	double v0z = sqrt(g*centre.z / (scale * (1 + scale / (2 * g))));
	double proportionalityConstant = g/(scale * v0z);
	return v3d(centre.x * proportionalityConstant, centre.y * proportionalityConstant, v0z);
}

// Cheap approximate Normal(0, 1) noise.
double NormalNoise()
{
	double result = -6;
	for (int i = 1; i <= 12; ++i)
		result += (double)rand() / RAND_MAX;
	return result;
}

// After a launch velocity is chosen, add some noise to it.
v3d Noisyv3d(v3d a, double sigma)
{
	return v3d(a.x + sigma*NormalNoise(), a.y + sigma*NormalNoise(),
		a.z + sigma*NormalNoise());
}

// How far have we landed from the target centre?
double SquaredDistance(v2d u, v2d v)
{
	return pow(u.x - v.x, 2) + pow(u.y - v.y, 2);
}

// A target is a horizontal hoop with a centre at z > 0. Did we land in it?
bool Landed(v3d x0, v3d v0, v3d centre, double radius, double g = 9.81)
{
	double cz = centre.z;
	return cz > 0 && pow(radius, 2) > SquaredDistance(Flatten(centre), LandingSite(x0 - cz, v0, g));
}

// Throw numScenarios times. How many shots landed?
int MonteCarlo(v3d x0, double sigmax, v3d v0, double sigmav, v3d centre, double radius, double g = 9.81, int numScenarios = 10000)
{
	int result = 0;
	for (int i = 1; i <= numScenarios; ++i)
	{
		if (Landed(Noisyv3d(x0, sigmax), Noisyv3d(v0, sigmav), centre, radius, g))
			result++;
	}
	return result;
}

// Play one game (well, numScenarios games).
void OneGame(double g = 9.81, double sigma = 1, int numScenarios = 10000)
{
	v3d x0 = v3d(0, 0, 0);
	double xOfCentre = NormalNoise();
	double yOfCentre = NormalNoise();
	double zOfCentre = abs(NormalNoise());
	v3d centre = v3d(xOfCentre, yOfCentre, zOfCentre);
	std::cout << "OK: the hoop centre is at x = " << xOfCentre << ", y = " << yOfCentre << ", z = " << zOfCentre << ".\n";
	double vx;
	double vy;
	double vz;
	std::cout << "You'll throw from the origin. What will be the ball's initial velocity's x-coordinate?\n";
	std::cin >> vx;
	std::cout << "And its y-coordinate?\n";
	std::cin >> vy;
	std::cout << "And its z-coordinate?\n";
	std::cin >> vz;
	v3d v0 = vz < 0 ? HintV0(centre, g, 0.1) : v3d(vx, vy, vz);
	std::cout << "Thanks! Your score is... ";
	int score = MonteCarlo(x0, sigma, v0, sigma, centre, 100, g, numScenarios);
	std::cout << score << "!\n";
}

int main()
{
	int numScenarios = 10000;
	std::cout << "Let's play basketball. We'll work in Cartesian coordinates, the z-axis vertical. I'll tell you where the hoop's centre is, then ask you for the initial velocity to launch the ball. Since you can't throw that precisely, you'll throw " << numScenarios << " times with some imprecision, scoring 1 point each time.\n";
	std::cout << "(Yes, I know basketball has a more complicated scoring system than that. I hope you don't mind.)\n";
	std::cout << "Oh, one more thing: if you haven't a clue what velocity to choose, give your answer a negative z-coordinate. In that case, I'll choose a good answer for you.\n"; 
	OneGame();
	double gs;
	std::cout << "That game assumed Earth's gravity, 1g. Now let's go to another planet. How many gs would you like?\n";
	std::cin >> gs;
	if (gs < 0)
	{
		std::cout << "I'll ignore that minus sign!\n";
		gs *= -1;
	}
	if (gs == 0)
	{
		std::cout << "You can't have 0g in this game. I'll assume you meant 1g.\n";
		gs = 1.0;
	}
	double sigma;
	std::cout << "Oh, one more question. Your imprecision is 1 unit. Maybe increasing or reducing it will help you. What imprecision would you like? (I'll ignore a sign. I'll also interpret 0 as 1.)\n";
	std::cin >> sigma;
	std::cout << "Starting alien game now!\n";
	OneGame(9.81 * gs, sigma == 0 ? 1.0 : abs(sigma));
	std::cout << "Hope you had fun. Feel free to play again by restarting this software.\n";
	system("PAUSE");
	return 0;
}

// Fin.
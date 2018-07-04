#include <iostream>
#include <matplotlibcpp.h>
#include <algorithm>
#include <random>
#include <zip.h>
#include <enumerate.h>

// NEAT
#include "neat.h"
#include "network.h"
#include "population.h"
#include "organism.h"
#include "genome.h"
#include "species.h"

namespace plt = matplotlibcpp;
using namespace std;
using namespace fpp;

const int WorldSizeX = 20;
const int WorldSizeY = 20;

// Need to separate them for the plot
vector<int> food_x(10);
vector<int> food_y(10);
vector<int> harm_x(10);
vector<int> harm_y(10);

enum CellType { EmptyCell = 0, FoodCell, HarmCell,  };
CellType World[WorldSizeX][WorldSizeY] = {};
mt19937 rng(42);

struct Individual
{
	float Life = 100;
	float AccumulatedLife = 0;
	int PosX = WorldSizeX / 2;
	int PosY = WorldSizeY / 2;

	void Step()
	{
		if (Life <= 0) return;
		// Simple solution to the bounds issue
		//	Kill everyone that steps out of the board!
		if (PosX < 1 || PosX > WorldSizeX - 2 || PosY < 1 || PosY > WorldSizeY - 2)
		{
			Life = 0;
			return;
		}

		// Read the 4 cells around the organism
		double sensors[4];
		sensors[0] = World[PosX    ][PosY + 1];
		sensors[1] = World[PosX    ][PosY - 1];
		sensors[2] = World[PosX + 1][PosY    ];
		sensors[3] = World[PosY - 1][PosY    ];

		// Load them into the network and compute output
		Brain->load_sensors(sensors);

		// This was taken directly from the NEAT source
		//		"use depth to ensure relaxation"
		bool success = Brain->activate();
		/*for (int relax = 0; relax <= Brain->max_depth(); relax++)
			success = Brain->activate();*/

		// Select the action to do using the activations as probabilites
		double activations[4];
		for (auto[idx, v] : enumerate(Brain->outputs))
			activations[idx] = max(0.0,v->activation);

		discrete_distribution<int> action_dist(begin(activations), end(activations));
		int action = action_dist(rng);
		//Brain->flush();

		// Execute action
		if (action == 0)
			PosX++;
		else if (action == 1)
			PosX--;
		else if (action == 2)
			PosY++;
		else if (action == 3)
			PosY--;
		
		// Update life based on current cell
		switch (World[PosX][PosY])
		{
		case FoodCell:
			Life += 10;
			break;
		case HarmCell:
			Life -= 20;
			break;
		}

		// Loose some fixed amount of life each turn
		Life -= 15;

		// Finally accumulate life
		AccumulatedLife += Life;
	}

	NEAT::Network * Brain = nullptr;
};

void BuildWorld()
{
	for (auto& x : food_x)
		x = uniform_int_distribution<>(0, WorldSizeX - 1)(rng);
	for (auto& y : food_y)
		y = uniform_int_distribution<>(0, WorldSizeY - 1)(rng);
	for (auto& x : harm_x)
		x = uniform_int_distribution<>(0, WorldSizeX - 1)(rng);
	for (auto& y : harm_y)
		y = uniform_int_distribution<>(0, WorldSizeY - 1)(rng);

	for (auto[x, y] : zip(food_x, food_y))
		World[x][y] = FoodCell;
	for (auto[x, y] : zip(harm_x, harm_x))
		World[x][y] = HarmCell;
}

float EvaluteNEATOrganism(NEAT::Organism * Org)
{
	Individual tmp_org;
	tmp_org.Brain = Org->net;

	const int max_iters = 2000;
	for (int i = 0; i < max_iters; i++)
	{
		if (tmp_org.Life <= 0) break;

		tmp_org.Step();
	}

	return tmp_org.AccumulatedLife;
}

int main()
{	
	// Load NEAT parameters
	NEAT::load_neat_params("../NEAT/test.ne");

	// Create population
	// 4 inputs, 4 outputs
	auto start_genome = new NEAT::Genome(4, 4, 0, 0);
	auto pop = new NEAT::Population(start_genome, NEAT::pop_size);

	// Run evolution
	for (int i = 0;i < 100;i++)
	{
		// Evaluate organisms
		for (auto& org : pop->organisms)
		{
			for (int j = 0; j < 10; j++)
			{
				// Repeat the evaluation a few times
				BuildWorld();

				org->fitness += EvaluteNEATOrganism(org);
			}
		}
		
		// Finally turn to the next generation
		pop->epoch(i + 1);
	}

	// Simulate and plot positions for the best organism
	plt::ion();

	double best_fitness = 0;
	Individual best_org;
	for (auto org : pop->organisms)
	{
		for (int j = 0; j < 10; j++)
		{
			// Repeat the evaluation a few times
			BuildWorld();

			org->fitness += EvaluteNEATOrganism(org);
		}

		if (org->fitness >= best_fitness)
		{
			best_fitness = org->fitness;
			best_org.Brain = org->net;
		}
	}

	while (true)
	{
		BuildWorld();

		best_org.Life = 100;
		best_org.PosX = WorldSizeX / 2;
		best_org.PosY = WorldSizeY / 2;

		while (best_org.Life > 0)
		{
			//BuildWorld();

			best_org.Step();
			cout << best_org.Life << endl;

			plt::clf();

			plt::plot(food_x, food_y, "bo");
			plt::plot(harm_x, harm_y, "ro");
			plt::plot({ (float)best_org.PosX }, { (float)best_org.PosY }, "xk");

			plt::xlim(0, WorldSizeX - 1);
			plt::ylim(0, WorldSizeY - 1);

			plt::pause(0.01);
		}
	}

	// TODO : CLEAR EVERYTHING HERE!

	return 0;
}
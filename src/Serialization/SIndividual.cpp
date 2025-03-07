#include "SIndividual.h"

#include <unordered_set>
#include <vector>

#include "../Evolution/Individual.h"
#include "enumerate.h"
#include "zip.h"

//NEAT
#include "genome.h"

using namespace std;
using namespace agio;
using namespace fpp;

SIndividual::SIndividual()
{
	RNG = minstd_rand(chrono::high_resolution_clock::now().time_since_epoch().count());
}

SIndividual::SIndividual(NEAT::Genome *genome, MorphologyTag morphology)
{
    this->morphologyTag = move(morphology);

    for(auto const&[key, param] : genome->MorphParams)
		parameters[key] = { param.ID, param.Value };

    brain = move(SNetwork(genome->genesis(genome->genome_id)));

    // Reconstruct actions and sensors
    TagDesc tag_desc(morphologyTag);
    Actions = tag_desc.ActionIDs;
    Sensors = tag_desc.SensorIDs;
	for (auto[idx, id] : enumerate(Sensors))
		SensorsMap[id] = idx;
	for (auto[idx, id] : enumerate(Actions))
		ActionsMap[id] = idx;

	ActivationsBuffer.resize(Actions.size());
	SensorsValues.resize(Sensors.size());
}

const MorphologyTag& SIndividual::GetMorphologyTag() const
{
    return morphologyTag;
}

void SIndividual::Reset()
{
    brain.flush();
    Interface->ResetState(State);
}

const std::unordered_map<int, Parameter>& SIndividual::GetParameters() const
{
    return parameters;
}

int SIndividual::DecideAction()
{
	// Send sensors to brain and activate
	brain.load_sensors(SensorsValues);
	bool sucess = brain.activate(); // TODO : Handle failure

	// Select action based on activations
	float act_sum = 0;
	for (auto[idx, node_idx] : enumerate(brain.outputs))
		act_sum += ActivationsBuffer[idx] = brain.all_nodes[node_idx].activation; // The activation function is in [0, 1], check line 461 of neat.cpp

	int action;

	if (UseMaxNetworkOutput)
	{
		float max_v = ActivationsBuffer[0];
		action = 0;

		// Yeah, I know that I'm checking the first element twice, but the performance impact is negligible
		for (auto[idx, v] : enumerate(ActivationsBuffer))
		{
			if (v > max_v)
			{
				max_v = v;
				action = idx;
			}
		}
	}
	else
	{
		if (act_sum > 1e-6)
		{
			// Generate the PCF
			ActivationsBuffer[0] /= act_sum;
			for (int idx = 1; idx < ActivationsBuffer.size(); ++idx)
				ActivationsBuffer[idx] = ActivationsBuffer[idx] / act_sum + ActivationsBuffer[idx - 1];

			// Sample the action distribution
			float px = generate_canonical<float, sizeof(float) * 8>(RNG);
			const auto firstIter = ActivationsBuffer.begin();
			const auto positionIter = lower_bound(firstIter, ActivationsBuffer.end(), px);
			action = static_cast<int>(positionIter - firstIter);
		}
		else
		{
			// Can't decide on an action because all activations are 0
			// Select one action at random
			uniform_int_distribution<int> action_dist(0, Actions.size() - 1);
			action = action_dist(RNG);
		}

	}

	return action;
}

void SIndividual::DecideAndExecute(void *World, const std::vector<BaseIndividual *> &Individuals)
{
    // Load sensors
    for (auto[value, idx] : zip(SensorsValues, Sensors))
        value = Interface->GetSensorRegistry()[idx].Evaluate(State, Individuals, this, World);

	// Decide action
	int action = DecideAction();

    // Finally execute the action
    Interface->GetActionRegistry()[Actions[action]].Execute(State, Individuals, this, World);
}

size_t SIndividual::TotalSize() const
{
	// Start with the size of the class
	size_t ret = sizeof(*this);
	
	// Add the size of the elements
	ret += ActivationsBuffer.capacity() * sizeof(float);
	ret += Actions.capacity() * sizeof(int);
	ret += Sensors.capacity() * sizeof(int);
	ret += morphologyTag.capacity() * sizeof(ComponentRef);
	// Note : rough lower bound, it's just considering the allocated elements
	ret += parameters.size() * sizeof(Parameter);
	
	return ret + brain.TotalSize();
}
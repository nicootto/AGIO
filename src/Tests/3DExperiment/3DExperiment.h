#pragma once
#include "../../Evolution/Globals.h"
#include "../../Utils/Math/Float2.h"
#include <vector>
#include <random>
#include "Public.h"

namespace agio
{
	class ExperimentInterface : public PublicInterface
	{
		std::minstd_rand0 RNG;
	public:
		int CurrentGenNumber = 0;

		ExperimentInterface();

		WorldData World;

		virtual ~ExperimentInterface() override {};

		virtual void Init() override;
		virtual void * MakeState(const class BaseIndividual * org) override;
		virtual void ResetState(void * State) override;
		virtual void DestroyState(void * State) override;
		virtual void * DuplicateState(void * State) override;
		virtual void ComputeFitness(const std::vector<class BaseIndividual*>& Individuals, void * World) override;
	};

	struct ExperimentParams
	{
		static int MaxSimulationSteps;
		static int PopSizeMultiplier;
		static int SimulationSize;
		static int GenerationsCount;
	};
}

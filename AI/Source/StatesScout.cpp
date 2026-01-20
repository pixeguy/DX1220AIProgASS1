#include "StatesScout.h"
#include "SceneData.h"
#include "GridSettings.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
StateScoutHealthy::StateScoutHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateScoutHealthy::~StateScoutHealthy()
{
}

void StateScoutHealthy::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->nearest = NULL;
	go->actionSpeed = 0.06;
}

void StateScoutHealthy::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
}

void StateScoutHealthy::Exit()
{
}
#pragma endregion

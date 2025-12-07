#include "StatesMortar.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
StateMortarHealthy::StateMortarHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateMortarHealthy::~StateMortarHealthy()
{
}

void StateMortarHealthy::Enter()
{
	go->moveSpeed = 0;
	go->moving = false;
	go->actionSpeed = 0.05;
	go->target = go->pos;
	go->nearest = NULL;
	go->energy = 0;
}

void StateMortarHealthy::Update(double dt)
{
	if (go->health < 40) {
		go->sm->SetNextState("Panic");
		return;
	}
}

void StateMortarHealthy::Exit()
{
}
#pragma endregion

#pragma region panic state
StateMortarPanic::StateMortarPanic(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMortarPanic::~StateMortarPanic()
{
}

void StateMortarPanic::Enter()
{
	go->moveSpeed = 0;
	go->moving = false;
	go->actionSpeed = 0.08;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateMortarPanic::Update(double dt)
{
	if (go->health <=0) {
		go->sm->SetNextState("Death");
		return;
	}

	if (go->health >= 50) {
		go->sm->SetNextState("Healthy");
		return;
	}
}

void StateMortarPanic::Exit()
{
}
#pragma endregion

#pragma region Death state
StateMortarDeath::StateMortarDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMortarDeath::~StateMortarDeath()
{
}

void StateMortarDeath::Enter()
{
	PostOffice::GetInstance()->Send("Scene", new MessageSpawnBigMorBomb(go));	
	go->moveSpeed = 0;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->active = false;
	go->type = GameObject::GO_NONE;
}

void StateMortarDeath::Update(double dt)
{
}

void StateMortarDeath::Exit()
{
}
#pragma endregion
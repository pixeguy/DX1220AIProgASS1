#ifndef STATES_RANGED_H
#define STATES_RANGED_H

#include "State.h"
#include "GameObject.h"

class StateRangedHealthy : public State
{
	GameObject *go;
public:
	StateRangedHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateRangedHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateRangedHurt : public State
{
	GameObject* go;
public:
	StateRangedHurt(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateRangedHurt();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateRangedPanic : public State
{
	GameObject* go;
public:
	StateRangedPanic(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateRangedPanic();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateRangedNearDeath : public State
{
	GameObject* go;
public:
	StateRangedNearDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateRangedNearDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
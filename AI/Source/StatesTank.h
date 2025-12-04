#ifndef STATES_TANK_H
#define STATES_TANK_H

#include "State.h"
#include "GameObject.h"

class StateTankHealthy : public State
{
	GameObject *go;
public:
	StateTankHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateTankHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateTankSoloHealthy : public State
{
	GameObject* go;
public:
	StateTankSoloHealthy(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateTankSoloHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
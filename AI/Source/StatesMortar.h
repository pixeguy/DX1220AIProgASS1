#ifndef STATES_MORTAR_H
#define STATES_MORTAR_H

#include "State.h"
#include "GameObject.h"

class StateMortarHealthy : public State
{
	GameObject *go;
public:
	StateMortarHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateMortarHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMortarPanic : public State
{
	GameObject* go;
public:
	StateMortarPanic(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMortarPanic();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMortarDeath : public State
{
	GameObject* go;
public:
	StateMortarDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMortarDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
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

class StateTankSuicide : public State
{
	GameObject* go;
public:
	StateTankSuicide(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateTankSuicide();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateTankDeath : public State
{
	GameObject* go;
public:
	StateTankDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateTankDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};


#endif
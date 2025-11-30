#ifndef STATES_ATTACKER_H
#define STATES_ATTACKER_H

#include "State.h"
#include "GameObject.h"

class StateAttackerHealthy : public State
{
	GameObject *go;
public:
	StateAttackerHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateAttackerHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAttackerStayStrong : public State
{
	GameObject* go;
public:
	StateAttackerStayStrong(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAttackerStayStrong();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAttackerFlee : public State
{
	GameObject* go;
public:
	StateAttackerFlee(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAttackerFlee();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAttackerNearDeath : public State
{
	GameObject* go;
public:
	StateAttackerNearDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAttackerNearDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAttackerDead : public State
{
	GameObject* go;
public:
	StateAttackerDead(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAttackerDead();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
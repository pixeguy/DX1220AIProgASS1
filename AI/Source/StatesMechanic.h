#ifndef STATES_MECHANIC_H
#define STATES_MECHANIC_H

#include "State.h"
#include "GameObject.h"

class StateMechanicHealthy : public State
{
	GameObject *go;
public:
	StateMechanicHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateMechanicHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMechanicHealing : public State
{
	GameObject* go;
public:
	StateMechanicHealing(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicHealing();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMechanicBuilding : public State
{
	GameObject* go;
public:
	StateMechanicBuilding(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicBuilding();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMechanicHurt : public State
{
	GameObject* go;
public:
	StateMechanicHurt(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicHurt();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMechanicHiding : public State
{
	GameObject* go;
public:
	StateMechanicHiding(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicHiding();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateMechanicGoldenChase : public State
{
	GameObject* go;
public:
	StateMechanicGoldenChase(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicGoldenChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();

	bool chasingPhase = true;
};

class StateMechanicDeath : public State
{
	GameObject* go;
public:
	StateMechanicDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateMechanicDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();

	bool chasingPhase = true;
};

#endif
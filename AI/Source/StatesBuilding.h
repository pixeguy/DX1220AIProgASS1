#ifndef STATES_BUILDING_H
#define STATES_BUILDING_H

#include "State.h"
#include "GameObject.h"

class StateBuildingHealthy : public State
{
	GameObject *m_go;
public:
	StateBuildingHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateBuildingHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBuildingDeath : public State
{
	GameObject* m_go;
public:
	StateBuildingDeath(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBuildingDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
#ifndef STATES_SCOUT_H
#define STATES_SCOUT_H

#include "State.h"
#include "GameObject.h"

class StateScoutHealthy : public State
{
	GameObject *go;
public:
	StateScoutHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateScoutHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
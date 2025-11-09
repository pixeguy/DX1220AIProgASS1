#ifndef STATES_RANGED_H
#define STATES_RANGED_H

#include "State.h"
#include "GameObject.h"

class StateRangedHealthy : public State
{
	GameObject *m_go;
public:
	StateRangedHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateRangedHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
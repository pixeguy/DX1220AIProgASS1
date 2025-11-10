#ifndef STATES_ATTACKER_H
#define STATES_ATTACKER_H

#include "State.h"
#include "GameObject.h"

class StateAttackerHealthy : public State
{
	GameObject *m_go;
public:
	StateAttackerHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateAttackerHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
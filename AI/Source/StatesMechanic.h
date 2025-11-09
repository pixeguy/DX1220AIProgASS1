#ifndef STATES_MECHANIC_H
#define STATES_MECHANIC_H

#include "State.h"
#include "GameObject.h"

class StateMechanicHealthy : public State
{
	GameObject *m_go;
public:
	StateMechanicHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateMechanicHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
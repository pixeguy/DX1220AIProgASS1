#ifndef STATES_MORTAR_H
#define STATES_MORTAR_H

#include "State.h"
#include "GameObject.h"

class StateMortarHealthy : public State
{
	GameObject *m_go;
public:
	StateMortarHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateMortarHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
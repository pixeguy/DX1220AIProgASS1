#ifndef STATES_SUPPORT_H
#define STATES_SUPPORT_H

#include "State.h"
#include "GameObject.h"

class StateSupportHealthy : public State
{
	GameObject *m_go;
public:
	StateSupportHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateSupportHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
#ifndef STATES_TANK_H
#define STATES_TANK_H

#include "State.h"
#include "GameObject.h"

class StateTankHealthy : public State
{
	GameObject *m_go;
public:
	StateTankHealthy(const std::string &stateID, GameObject *go = NULL);
	virtual ~StateTankHealthy();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

#endif
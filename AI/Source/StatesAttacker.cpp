#include "StatesAttacker.h"
#include "SceneData.h"


StateAttackerHealthy::StateAttackerHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerHealthy::~StateAttackerHealthy()
{
}

void StateAttackerHealthy::Enter()
{
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateAttackerHealthy::Update(double dt)
{
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = true;
	if (m_go->nearest)
	{
		if (m_go->nearest->pos.x > m_go->pos.x)
			m_go->moveLeft = false;
		else
			m_go->moveRight = false;
		if (m_go->nearest->pos.y > m_go->pos.y)
			m_go->moveDown = false;
		else
			m_go->moveUp = false;
	}
}

void StateAttackerHealthy::Exit()
{
}

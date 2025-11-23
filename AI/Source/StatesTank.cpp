#include "StatesTank.h"
#include "SceneData.h"


StateTankHealthy::StateTankHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateTankHealthy::~StateTankHealthy()
{
}

void StateTankHealthy::Enter()
{
	m_go->moveSpeed = 1;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateTankHealthy::Update(double dt)
{
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->nearest->pos.x - m_go->pos.x;
		float diffY = m_go->nearest->pos.y - m_go->pos.y;
		if (fabs(diffX) > fabs(diffY))
		{
			if (diffX > 0)
				m_go->moveRight = true;
			else
				m_go->moveLeft = true;
		}
		else
		{
			if (diffY > 0)
				m_go->moveUp = true;
			else
				m_go->moveDown = true;
		}
	}
}

void StateTankHealthy::Exit()
{
}

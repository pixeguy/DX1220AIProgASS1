#include "StatesSupport.h"
#include "SceneData.h"


StateSupportHealthy::StateSupportHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateSupportHealthy::~StateSupportHealthy()
{
}

void StateSupportHealthy::Enter()
{
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateSupportHealthy::Update(double dt)
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

void StateSupportHealthy::Exit()
{
}

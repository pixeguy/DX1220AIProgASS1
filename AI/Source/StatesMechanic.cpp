#include "StatesMechanic.h"
#include "SceneData.h"


StateMechanicHealthy::StateMechanicHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateMechanicHealthy::~StateMechanicHealthy()
{
}

void StateMechanicHealthy::Enter()
{
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateMechanicHealthy::Update(double dt)
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

	//if close enough to a spawner
	float distance = 999;
	if(m_go->nearest != NULL)
	{ 
		distance = (m_go->pos - m_go->nearest->pos).Length();
	}
	if (distance < 5) //grid size is 5
	{
		GameObject* go2 = m_go->nearest;
		m_go->moveSpeed = 0;
		if (m_go->EnergyReduce(1))
		{

			if (go2->woodenLogs >= go2->metalParts * 2)
			{
				go2->metalParts += 1;
			}
			else
			{
				go2->woodenLogs += 1;
			}
		}
	}
}

void StateMechanicHealthy::Exit()
{
}

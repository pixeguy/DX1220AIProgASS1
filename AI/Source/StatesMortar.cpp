#include "StatesMortar.h"
#include "SceneData.h"


StateMortarHealthy::StateMortarHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateMortarHealthy::~StateMortarHealthy()
{
}

void StateMortarHealthy::Enter()
{
	m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateMortarHealthy::Update(double dt)
{
	//m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = true;
	//if (m_go->nearest)
	//{
	//	if (m_go->normalTarget.x > m_go->pos.x)
	//		m_go->moveLeft = false;
	//	else
	//		m_go->moveRight = false;
	//	if (m_go->normalTarget.y > m_go->pos.y)
	//		m_go->moveDown = false;
	//	else
	//		m_go->moveUp = false;
	//}
}

void StateMortarHealthy::Exit()
{
}

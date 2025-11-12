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
	m_go->moveSpeed = 1;
	m_go->actionSpeed = 1;
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

StateSupportDeath::StateSupportDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateSupportDeath::~StateSupportDeath()
{
}

void StateSupportDeath::Enter()
{
	if (m_go->nearest != NULL) {
		m_go->nearest->supportSpeed = 0;
	}
	m_go->type = GameObject::GO_NONE;
	m_go->active = false;
}

void StateSupportDeath::Update(double dt)
{

}

void StateSupportDeath::Exit()
{
}







StateNone::StateNone(const std::string& stateID, const std::string& nextState, GameObject* go)
	: State(stateID),
	m_go(go)
{
	this->nextState = nextState;
}

StateNone::~StateNone()
{
}

void StateNone::Enter()
{
}

void StateNone::Update(double dt)
{
	m_go->sm->SetNextState(nextState);
}

void StateNone::Exit()
{
}
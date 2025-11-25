#include "StatesRanged.h"
#include "SceneData.h"

#pragma region healthy state
StateRangedHealthy::StateRangedHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateRangedHealthy::~StateRangedHealthy()
{
}

void StateRangedHealthy::Enter()
{
	m_go->moveSpeed = 0.75;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateRangedHealthy::Update(double dt)
{
	if (m_go->health < 80) {
		m_go->sm->SetNextState("Hurt");
		return;
	}
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;
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

void StateRangedHealthy::Exit()
{
}
#pragma endregion

#pragma region hurt state
StateRangedHurt::StateRangedHurt(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateRangedHurt::~StateRangedHurt()
{
}

void StateRangedHurt::Enter()
{
	m_go->moveSpeed = 0.75;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateRangedHurt::Update(double dt)
{
	if (m_go->health < 50) {
		m_go->sm->SetNextState("Panic");
		return;
	}
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;
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

void StateRangedHurt::Exit()
{
}

#pragma endregion

#pragma region panic state
StateRangedPanic::StateRangedPanic(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateRangedPanic::~StateRangedPanic()
{
}

void StateRangedPanic::Enter()
{
	m_go->moveSpeed = 0.75;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateRangedPanic::Update(double dt)
{
	if (m_go->health < 20) {
		m_go->sm->SetNextState("NearDeath");
		return;
	}
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;
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

void StateRangedPanic::Exit()
{
}

#pragma endregion

#pragma region near death state
StateRangedNearDeath::StateRangedNearDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateRangedNearDeath::~StateRangedNearDeath()
{
}

void StateRangedNearDeath::Enter()
{
	m_go->moveSpeed = 0.75;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateRangedNearDeath::Update(double dt)
{
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;
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

void StateRangedNearDeath::Exit()
{
}

#pragma endregion
#include "StatesBuilding.h"
#include "SceneData.h"


StateBuildingHealthy::StateBuildingHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateBuildingHealthy::~StateBuildingHealthy()
{
}

void StateBuildingHealthy::Enter()
{
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateBuildingHealthy::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
	}
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	//if (SceneData::GetInstance()->GetFishCount() < 12)
	//	m_go->sm->SetNextState("Naughty");
}

void StateBuildingHealthy::Exit()
{
}

StateBuildingDeath::StateBuildingDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateBuildingDeath::~StateBuildingDeath()
{
}

void StateBuildingDeath::Enter()
{
	m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.3;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
	m_go->active = false;
	m_go->type = GameObject::GO_NONE;
}

void StateBuildingDeath::Update(double dt)
{
}

void StateBuildingDeath::Exit()
{
}

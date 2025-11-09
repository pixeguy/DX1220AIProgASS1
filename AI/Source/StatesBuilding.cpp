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
	m_go->moveSpeed = 0;
	m_go->target = m_go->pos;
	//if (SceneData::GetInstance()->GetFishCount() < 12)
	//	m_go->sm->SetNextState("Naughty");
}

void StateBuildingHealthy::Exit()
{
}

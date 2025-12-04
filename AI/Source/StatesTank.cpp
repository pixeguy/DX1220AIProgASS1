#include "StatesTank.h"
#include "SceneData.h"
#include "GridSettings.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
StateTankHealthy::StateTankHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateTankHealthy::~StateTankHealthy()
{
}

void StateTankHealthy::Enter()
{
	go->moveSpeed = 1;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateTankHealthy::Update(double dt)
{
	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	if (go->nearest)
	{
		float diffX = go->normalTarget.x - go->pos.x;
		float diffY = go->normalTarget.y - go->pos.y;
		if (fabs(diffX) > fabs(diffY))
		{
			if (diffX > 0)
				go->moveRight = true;
			else
				go->moveLeft = true;
		}
		else
		{
			if (diffY > 0)
				go->moveUp = true;
			else
				go->moveDown = true;
		}
	}

	if (go->external2 != NULL && go->external2->active)
	{
		if ((go->external2->pos - go->pos).Length() < m_gridSize * 5)
		{
			float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
			if (go->EnergyReduce(finalActionSpeed))
			{
				//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
				PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
			}
		}
	}
}

void StateTankHealthy::Exit()
{
}
#pragma endregion

#pragma region solo healthy state
StateTankSoloHealthy::StateTankSoloHealthy(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateTankSoloHealthy::~StateTankSoloHealthy()
{
}

void StateTankSoloHealthy::Enter()
{
	go->moveSpeed = 1;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateTankSoloHealthy::Update(double dt)
{
	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	if (go->nearest)
	{
		float diffX = go->nearest->pos.x - go->pos.x;
		float diffY = go->nearest->pos.y - go->pos.y;
		if (fabs(diffX) > fabs(diffY))
		{
			if (diffX > 0)
				go->moveRight = true;
			else
				go->moveLeft = true;
		}
		else
		{
			if (diffY > 0)
				go->moveUp = true;
			else
				go->moveDown = true;
		}
	}

	if (go->nearest != NULL && go->nearest->active)
	{
		if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
		{
			go->moving = false;
			float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
			if (go->EnergyReduce(finalActionSpeed))
			{
				//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
				PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
			}
		}
		else {
			go->moving = true;
		}
	}
}

void StateTankSoloHealthy::Exit()
{
}
#pragma endregion

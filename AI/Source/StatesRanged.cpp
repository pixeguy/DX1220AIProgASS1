#include "StatesRanged.h"
#include "SceneData.h"
#include "GridSettings.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
StateRangedHealthy::StateRangedHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateRangedHealthy::~StateRangedHealthy()
{
}

void StateRangedHealthy::Enter()
{
	go->moveSpeed = 0.75;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateRangedHealthy::Update(double dt)
{
	if (go->health < 80) {
		go->sm->SetNextState("Hurt");
		return;
	}
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


	{
		if (go->nearest != NULL && go->nearest->active != false && !go->nearest->hiding)
		{
			if (go->countDown < m_gridSize * 2) //if target too close
			{
				//move away
				//go->ignoreCurMove = true;
				go->moving = true;
				int redOrBlue = (go->side == GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->pos + Vector3(redOrBlue * 3 * m_gridSize, 0, 0);
				go->target = go->pos + Vector3(redOrBlue * m_gridSize, 0, 0);
				if (go->normalTarget.x < 0 || go->normalTarget.x > m_noGrid * m_gridSize || go->normalTarget.y < 0 || go->normalTarget.y > m_noGrid * m_gridSize)
				{
					go->normalTarget = go->pos;
					go->target = go->pos;
				}

			}
			else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
			{
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(go, GameObject::GO_FISHFOOD, 2, range));
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
				}
			}
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
	go(go)
{
}

StateRangedHurt::~StateRangedHurt()
{
}

void StateRangedHurt::Enter()
{
	go->moveSpeed = 0.75;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateRangedHurt::Update(double dt)
{
	if (go->health < 50) {
		go->sm->SetNextState("Panic");
		return;
	}
	if (go->health > 90)
	{
		go->sm->SetNextState("Healthy");
		return;
	}
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

	{
		if (go->nearest != NULL && go->nearest->active != false && !go->nearest->hiding)
		{
			if (go->countDown < m_gridSize * 3) //if target too close
			{
				go->moving = true;
				int redOrBlue = (go->side == GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->pos + Vector3(redOrBlue * 3 * m_gridSize, 0, 0);
				go->target = go->pos + Vector3(redOrBlue * m_gridSize, 0, 0);
				if (go->normalTarget.x < 0 || go->normalTarget.x > m_noGrid * m_gridSize || go->normalTarget.y < 0 || go->normalTarget.y > m_noGrid * m_gridSize)
				{
					go->normalTarget = go->pos;
					go->target = go->pos;
				}
			}
			else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
			{
				//std::cout << "shooting!" << std::endl;
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
				}
			}
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
	go(go)
{
}

StateRangedPanic::~StateRangedPanic()
{
}

void StateRangedPanic::Enter()
{
	go->moveSpeed = 0.75;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateRangedPanic::Update(double dt)
{
	if (go->health < 20) {
		go->sm->SetNextState("NearDeath");
		return;
	}
	if (go->health > 60)
	{
		go->sm->SetNextState("Hurt");
		return;
	}
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

	{
		if (go->nearest != NULL && go->nearest->active != false && !go->nearest->hiding)
		{
			if (go->countDown < m_gridSize * 4) //if target too close
			{
				//std::cout << "running away!" << std::endl;
				//move away
				go->moving = true;
				int redOrBlue = (go->side == GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->pos + Vector3(redOrBlue * 3 * m_gridSize, 0, 0);
				go->target = go->pos + Vector3(redOrBlue * m_gridSize, 0, 0);
				if (go->normalTarget.x < 0 || go->normalTarget.x > m_noGrid * m_gridSize || go->normalTarget.y < 0 || go->normalTarget.y > m_noGrid * m_gridSize)
				{
					go->normalTarget = go->pos;
					go->target = go->pos;
				}
			}
			else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
			{
				//std::cout << "shooting!" << std::endl;
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
				}
			}
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
	go(go)
{
}

StateRangedNearDeath::~StateRangedNearDeath()
{
}

void StateRangedNearDeath::Enter()
{
	go->moveSpeed = 0.75;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
}

void StateRangedNearDeath::Update(double dt)
{
	if (go->health > 40)
	{
		go->sm->SetNextState("Panic");
		return;
	}
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
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

	{
		if (go->nearest != NULL && go->nearest->active != false && !go->nearest->hiding)
		{
			if (go->countDown < m_gridSize * 4) //if target too close
			{
				//std::cout << "running away!" << std::endl;
				//move away
				go->moving = true;
				int redOrBlue = (go->side== GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->pos + Vector3(redOrBlue * 3 * m_gridSize, 0, 0);
				go->target = go->pos + Vector3(redOrBlue * m_gridSize, 0, 0);
				if (go->normalTarget.x < 0 || go->normalTarget.x > m_noGrid * m_gridSize || go->normalTarget.y < 0 || go->normalTarget.y > m_noGrid * m_gridSize)
				{
					go->normalTarget = go->pos;
					go->target = go->pos;
				}
			}
			else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
			{
				//std::cout << "shooting!" << std::endl;
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					//PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProj(go));
				}
			}
		}
	}
}

void StateRangedNearDeath::Exit()
{
}

#pragma endregion

#pragma region death state
StateRangedDeath::StateRangedDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateRangedDeath::~StateRangedDeath()
{
}

void StateRangedDeath::Enter()
{
	go->moveSpeed = 0;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->active = false;
	go->type = GameObject::GO_NONE;
}

void StateRangedDeath::Update(double dt)
{

}

void StateRangedDeath::Exit()
{
}

#pragma endregion
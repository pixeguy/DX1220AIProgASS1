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
	go->actionSpeed = 0.1;
	go->target = go->pos;
	go->nearest = NULL;
	go->moving = true;
}

void StateTankHealthy::Update(double dt)
{
	if (go->health < 40)
	{
		go->actionSpeed = 0.19f;
		go->panicking = true;
	}
	else { go->actionSpeed = 0.1f; go->panicking = false;}

	if(go->health <= 0)
	{
		if (go->atkTarget == NULL || !go->atkTarget->active) { go->sm->SetNextState("Death"); return; }
		float random = Math::RandFloatMinMax(0.f, 1.f);
		if( random < 0.5f )
			go->sm->SetNextState("Death");
		else
			go->sm->SetNextState("Suicide");
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

	if (go->external2 != NULL && go->external2->active)
	{
		if ((go->external2->pos - go->pos).Length() < m_gridSize * 5)
		{
			float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
			if (go->EnergyReduce(finalActionSpeed))
			{
				if (go->external2->type != GameObject::GO_MAINBASE && go->external2->type != GameObject::GO_SPAWNER) {
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProjTank(go, go->external2, false));
				}
				else
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProjTank(go, go->external2, true));
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
	go->actionSpeed = 0.1;
	go->target = go->pos;
	go->nearest = NULL;
	go->moving = true;
}

void StateTankSoloHealthy::Update(double dt)
{
	if (go->health < 40)
	{
		go->actionSpeed = 0.15f;
		go->panicking = true;
	}
	else { go->actionSpeed = 0.1f; go->panicking = false; }
	if (go->health <= 0)
	{
		if (go->atkTarget == NULL || !go->atkTarget->active) { go->sm->SetNextState("Death"); return; }
		float random = Math::RandFloatMinMax(0.f, 1.f);
		if( random < 0.5f )
			go->sm->SetNextState("Death");
		else
			go->sm->SetNextState("Suicide");
			return;
	}
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
				if (go->nearest->type != GameObject::GO_MAINBASE && go->nearest->type != GameObject::GO_SPAWNER) {
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProjTank(go, go->nearest, false));
				}
				else
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnProjTank(go, go->nearest, true));
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

#pragma region suicide state
StateTankSuicide::StateTankSuicide(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateTankSuicide::~StateTankSuicide()
{
}

void StateTankSuicide::Enter()
{
	go->moveSpeed = 1.5;
	go->actionSpeed = 0.2;
	go->target = go->pos;
	go->nearest = NULL;
	go->countDown = 2;
	go->useMoves = 3;
	go->atkRange = 0;
	go->moving = true;
}

void StateTankSuicide::Update(double dt)
{
	//go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	//if (go->nearest)
	//{
	//	float diffX = go->nearest->pos.x - go->pos.x;
	//	float diffY = go->nearest->pos.y - go->pos.y;
	//	if (fabs(diffX) > fabs(diffY))
	//	{
	//		if (diffX > 0)
	//			go->moveRight = true;
	//		else
	//			go->moveLeft = true;
	//	}
	//	else
	//	{
	//		if (diffY > 0)
	//			go->moveUp = true;
	//		else
	//			go->moveDown = true;
	//	}
	//}

	//if (go->countDown > 0)
	//{
	//	go->countDown -= dt;
	//}
	//else {
	//	go->moveSpeed = 0;
	//	go->actionSpeed = 0.3;
	//	go->target = go->pos;
	//	go->nearest = NULL;
	//	go->active = false;
	//	go->type = GameObject::GO_NONE;
	//}
}

void StateTankSuicide::Exit()
{
}
#pragma endregion

#pragma region death state
StateTankDeath::StateTankDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateTankDeath::~StateTankDeath()
{
}

void StateTankDeath::Enter()
{
	PostOffice::GetInstance()->Send("Scene", new MessageSpawnAttacker(go));
	go->moveSpeed = 0;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->active = false;
	go->type = GameObject::GO_NONE;
}

void StateTankDeath::Update(double dt)
{
}

void StateTankDeath::Exit()
{
}
#pragma endregion
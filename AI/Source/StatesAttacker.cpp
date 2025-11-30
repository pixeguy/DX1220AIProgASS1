#include "StatesAttacker.h"
#include "SceneData.h"
#include "GridSettings.h"

#pragma region healthy state
StateAttackerHealthy::StateAttackerHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateAttackerHealthy::~StateAttackerHealthy()
{
}

void StateAttackerHealthy::Enter()
{
	go->moveSpeed = 1;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->external = NULL;
}

void StateAttackerHealthy::Update(double dt)
{
	if(go->health <= 70)
	{
		//float random = Math::RandFloatMinMax(0.f, 1.f);
		//if( random < 0.5f )
		//	go->sm->SetNextState("StayStrong");
		//else
		//	go->sm->SetNextState("Flee");
		go->sm->SetNextState("Flee");
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

	{
		if (go->nearest != NULL && go->nearest->active != false) {
			if ((go->nearest->pos - go->pos).Length() < m_gridSize)
			{
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					go->nearest->health -= 5;
					go->nearest->lastAttacker = go;
				}
			}
			else if (go->urgent) { //if im not close enough, continue checking
				go->moving = true;
				go->moveSpeed = 2;
			}
		}
	}
}

void StateAttackerHealthy::Exit()
{
}
#pragma endregion

#pragma region stay strong state
StateAttackerStayStrong::StateAttackerStayStrong(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateAttackerStayStrong::~StateAttackerStayStrong()
{
}

void StateAttackerStayStrong::Enter()
{
	//go->moveSpeed = 0;
	go->actionSpeed = 0.2;
	//go->target = go->pos;
	//go->nearest = NULL;
	//go->active = false;
	//go->type = GameObject::GO_NONE;
}

void StateAttackerStayStrong::Update(double dt)
{
	if (go->health <= 30)
	{
		go->sm->SetNextState("NearDeath");
		return;
	}
	if (go->health > 80)
	{
		go->sm->SetNextState("Healthy");
		return;
	}

	float random = Math::RandFloatMinMax(0.f, 1.f); //once ive chosen to stay, have lower chance to run
	//if (random < 0.3f)
	//	go->sm->SetNextState("Flee");

	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	if (go->nearest)
	{
		float diffX = go->normalTarget.x - go->pos.x;
		float diffY = go->normalTarget.y - go->pos.y;

		if (diffX == 0 && diffY == 0)
		{

		}
		else if (fabs(diffX) > fabs(diffY))
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
		if (go->nearest != NULL && go->nearest->active != false) {
			if ((go->nearest->pos - go->pos).Length() < m_gridSize)
			{
				go->moving = false;
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					go->nearest->health -= 5;
					go->nearest->lastAttacker = go;
				}
			}
			else if (go->urgent) { //if im not close enough, continue checking
				go->moving = true;
				go->moveSpeed = 2;
			}
		}
	}
}

void StateAttackerStayStrong::Exit()
{
}
#pragma endregion

#pragma region fleeing state
StateAttackerFlee::StateAttackerFlee(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateAttackerFlee::~StateAttackerFlee()
{
}

void StateAttackerFlee::Enter()
{
	//go->moveSpeed = 0;
	//go->actionSpeed = 0.3;
	go->nearest = NULL;
	go->external = NULL;
	//go->active = false;
	//go->type = GameObject::GO_NONE;
}

void StateAttackerFlee::Update(double dt)
{
	if (go->health <= 30)
	{
		go->sm->SetNextState("NearDeath");
		return;
	}
	if(go->health > 80)
	{
		go->sm->SetNextState("Healthy");
		return;
	}

	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	if (go->nearest)
	{
		float diffX = go->normalTarget.x - go->pos.x;
		float diffY = go->normalTarget.y - go->pos.y;

		if (diffX == 0 && diffY == 0)
		{
			
		}
		else if (fabs(diffX) > fabs(diffY))
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
		if (go->nearest != NULL && go->nearest->active != false) {
			float distToSpawner = (go->nearest->pos - go->pos).Length();
			bool closeToSpawner = distToSpawner < m_gridSize * 3;  // choose radius

			bool attackerLostSight =
				(go->lastAttacker != NULL &&
					go->lastAttacker->nearest != go);

			if (!closeToSpawner && attackerLostSight)
			{
				go->moving = false;
			}
			else
			{
				// run to spawner and pace behind it
				int redOrBlue = (go->type == GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->nearest->pos + Vector3(redOrBlue * 2.5f * m_gridSize,(go->steps * 0.5f) * m_gridSize,0);

				float distToSpot = (go->normalTarget - go->pos).Length();

				if (distToSpot < m_gridSize)
				{
					Vector3 paceOffset = Vector3(0, (-go->steps) * m_gridSize, 0);
					Vector3 paceTarget = go->normalTarget + paceOffset;

					if ((paceTarget - go->pos).Length() < m_gridSize)
					{
						go->steps *= -1;
						paceOffset = Vector3(0, (go->steps * 0.5f) * m_gridSize, 0);
						paceTarget = go->normalTarget + paceOffset;
					}

					go->moving = true;
					go->normalTarget = paceTarget;
				}
				else
				{
					go->moving = true;
				}
			}
		}
	}
}

void StateAttackerFlee::Exit()
{
}
#pragma endregion

#pragma region near death state
StateAttackerNearDeath::StateAttackerNearDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateAttackerNearDeath::~StateAttackerNearDeath()
{
}

void StateAttackerNearDeath::Enter()
{
	//go->moveSpeed = 0;
	//go->actionSpeed = 0.3;
	go->nearest = NULL;
	go->external = NULL;
	//go->active = false;
	//go->type = GameObject::GO_NONE;
}

void StateAttackerNearDeath::Update(double dt)
{
	if(go->health <= 0)
	{
		go->sm->SetNextState("Dead");
		return;
	}
	if(go->health > 40)
	{
		//float random = Math::RandFloatMinMax(0.f, 1.f);
		//if( random < 0.5f )
		//	go->sm->SetNextState("StayStrong");
		//else
		//	go->sm->SetNextState("Flee");
		go->sm->SetNextState("Flee");

		go->urgent = false;
		return;
	}

	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
	if (go->nearest)
	{
		float diffX = go->normalTarget.x - go->pos.x;
		float diffY = go->normalTarget.y - go->pos.y;

		if (diffX == 0 && diffY == 0)
		{

		}
		else if (fabs(diffX) > fabs(diffY))
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
		if (go->nearest != NULL && go->nearest->active != false) {
			float distToSpawner = (go->nearest->pos - go->pos).Length();
			bool closeToSpawner = distToSpawner < m_gridSize * 3;  // choose radius

			bool attackerLostSight =
				(go->lastAttacker != NULL &&
					go->lastAttacker->nearest != go);

			if (!closeToSpawner && attackerLostSight)
			{
				go->moving = false;
			}
			else
			{
				// run to spawner and pace behind it
				int redOrBlue = (go->type == GameObject::SIDE_BLUE) ? 1 : -1;
				go->normalTarget = go->nearest->pos +
					Vector3(redOrBlue * 2.5f * m_gridSize,
						(go->steps * 0.5f) * m_gridSize,
						0);

				float distToSpot = (go->normalTarget - go->pos).Length();

				if (distToSpot < m_gridSize)
				{
					Vector3 paceOffset = Vector3(0, (-go->steps) * m_gridSize, 0);
					Vector3 paceTarget = go->normalTarget + paceOffset;

					if ((paceTarget - go->pos).Length() < m_gridSize)
					{
						go->steps *= -1;
						paceOffset = Vector3(0, (go->steps * 0.5f) * m_gridSize, 0);
						paceTarget = go->normalTarget + paceOffset;
					}

					go->moving = true;
					go->normalTarget = paceTarget;
				}
				else
				{
					go->moving = true;
				}
			}
		}
	}
}

void StateAttackerNearDeath::Exit()
{
}
#pragma endregion

#pragma region dead state
StateAttackerDead::StateAttackerDead(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateAttackerDead::~StateAttackerDead()
{
}

void StateAttackerDead::Enter()
{
	go->moveSpeed = 0;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->active = false;
	go->type = GameObject::GO_NONE;
}

void StateAttackerDead::Update(double dt)
{

}

void StateAttackerDead::Exit()
{
}
#pragma endregion

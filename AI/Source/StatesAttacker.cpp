#include "StatesAttacker.h"
#include "SceneData.h"
#include "GridSettings.h"

#pragma region healthy state
StateAttackerHealthy::StateAttackerHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerHealthy::~StateAttackerHealthy()
{
}

void StateAttackerHealthy::Enter()
{
	m_go->moveSpeed = 1;
	m_go->actionSpeed = 0.3;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
	m_go->external = NULL;
}

void StateAttackerHealthy::Update(double dt)
{
	if(m_go->health <= 70)
	{
		//float random = Math::RandFloatMinMax(0.f, 1.f);
		//if( random < 0.5f )
		//	m_go->sm->SetNextState("StayStrong");
		//else
		//	m_go->sm->SetNextState("Flee");
		m_go->sm->SetNextState("Flee");
		return;
	}
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->nearest->pos.x - m_go->pos.x;
		float diffY = m_go->nearest->pos.y - m_go->pos.y;
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

	//if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize)
	//{
	//	m_go->moving = false;
	//	if (m_go->EnergyReduce(0.3f))
	//	{
	//		m_go->nearest->health -= 5;
	//	}
	//}
	//else { //if im not there yet, continue checking
	//	m_go->moving = true;
	//	MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
	//	Handle(&msgCheckEnemy);
	//}
}

void StateAttackerHealthy::Exit()
{
}
#pragma endregion

#pragma region stay strong state
StateAttackerStayStrong::StateAttackerStayStrong(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerStayStrong::~StateAttackerStayStrong()
{
}

void StateAttackerStayStrong::Enter()
{
	//m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.2;
	//m_go->target = m_go->pos;
	//m_go->nearest = NULL;
	//m_go->active = false;
	//m_go->type = GameObject::GO_NONE;
}

void StateAttackerStayStrong::Update(double dt)
{
	if (m_go->health <= 30)
	{
		m_go->sm->SetNextState("NearDeath");
		return;
	}
	if (m_go->health > 80)
	{
		m_go->sm->SetNextState("Healthy");
		return;
	}

	float random = Math::RandFloatMinMax(0.f, 1.f); //once ive chosen to stay, have lower chance to run
	//if (random < 0.3f)
	//	m_go->sm->SetNextState("Flee");

	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;

		if (diffX == 0 && diffY == 0)
		{

		}
		else if (fabs(diffX) > fabs(diffY))
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

void StateAttackerStayStrong::Exit()
{
}
#pragma endregion

#pragma region fleeing state
StateAttackerFlee::StateAttackerFlee(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerFlee::~StateAttackerFlee()
{
}

void StateAttackerFlee::Enter()
{
	//m_go->moveSpeed = 0;
	//m_go->actionSpeed = 0.3;
	m_go->nearest = NULL;
	m_go->external = NULL;
	//m_go->active = false;
	//m_go->type = GameObject::GO_NONE;
}

void StateAttackerFlee::Update(double dt)
{
	if (m_go->health <= 30)
	{
		m_go->sm->SetNextState("NearDeath");
		return;
	}
	if(m_go->health > 80)
	{
		m_go->sm->SetNextState("Healthy");
		return;
	}

	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;

		if (diffX == 0 && diffY == 0)
		{
			
		}
		else if (fabs(diffX) > fabs(diffY))
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

void StateAttackerFlee::Exit()
{
}
#pragma endregion

#pragma region near death state
StateAttackerNearDeath::StateAttackerNearDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerNearDeath::~StateAttackerNearDeath()
{
}

void StateAttackerNearDeath::Enter()
{
	//m_go->moveSpeed = 0;
	//m_go->actionSpeed = 0.3;
	m_go->nearest = NULL;
	m_go->external = NULL;
	//m_go->active = false;
	//m_go->type = GameObject::GO_NONE;
}

void StateAttackerNearDeath::Update(double dt)
{
	if(m_go->health <= 0)
	{
		m_go->sm->SetNextState("Dead");
		return;
	}
	if(m_go->health > 40)
	{
		//float random = Math::RandFloatMinMax(0.f, 1.f);
		//if( random < 0.5f )
		//	m_go->sm->SetNextState("StayStrong");
		//else
		//	m_go->sm->SetNextState("Flee");
		m_go->sm->SetNextState("Flee");

		m_go->urgent = false;
		return;
	}

	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = false;
	if (m_go->nearest)
	{
		float diffX = m_go->normalTarget.x - m_go->pos.x;
		float diffY = m_go->normalTarget.y - m_go->pos.y;

		std::cout << "diffX: " << diffX << " diffY: " << diffY << std::endl;
		if (diffX == 0 && diffY == 0)
		{

		}
		else if (fabs(diffX) > fabs(diffY))
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

void StateAttackerNearDeath::Exit()
{
}
#pragma endregion

#pragma region dead state
StateAttackerDead::StateAttackerDead(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateAttackerDead::~StateAttackerDead()
{
}

void StateAttackerDead::Enter()
{
	m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.3;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
	m_go->active = false;
	m_go->type = GameObject::GO_NONE;
}

void StateAttackerDead::Update(double dt)
{

}

void StateAttackerDead::Exit()
{
}
#pragma endregion

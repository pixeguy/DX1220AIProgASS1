#include "StatesSupport.h"
#include "SceneData.h"
#include "GridSettings.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
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
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateSupportHealthy::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
		m_go->sm->m_currState->Exit();
		m_go->sm->m_currState = m_go->sm->m_nextState;
		m_go->sm->m_currState->Enter();
		return;
	}
	if (m_go->alliesActiveCount == 0)
	{
		m_go->sm->SetNextState("Hiding");
		return;
	}
	if (m_go->health < 40)
	{
		m_go->sm->SetNextState("Hurt");
		return;
	}
	if (m_go->healTarget != NULL && m_go->healTarget->active == true) {
		if (m_go->urgent)
		{
			m_go->sm->SetNextState("UrgentHealing");
			return;
		}
		m_go->sm->SetNextState("Healing");
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
	if (m_go->nearest != NULL) {
		if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize * 5) {
			m_go->nearest->supportSpeed = 0.4;
			m_go->nearest->supportActionSpeed = 0.08;
			m_go->moving = false;
			/*std::cout << "followed something to close" << std::endl;*/
		}
		else { m_go->moving = true; m_go->nearest->supportSpeed = 0; m_go->nearest->supportActionSpeed = 0; /*std::cout << "following" << std::endl*/; }
	}
	else { m_go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
}

void StateSupportHealthy::Exit()
{
}
#pragma endregion

#pragma region death state
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
		m_go->nearest->supportActionSpeed = 0;
	}
	m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.3;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
	m_go->active = false;
	m_go->type = GameObject::GO_NONE;
}

void StateSupportDeath::Update(double dt)
{
	if (m_go->nearest != NULL) {
		m_go->nearest->supportSpeed = 0;
		m_go->nearest->supportActionSpeed = 0;
	}
	m_go->moveSpeed = 0;
	m_go->actionSpeed = 0.3;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
	m_go->active = false;
	m_go->type = GameObject::GO_NONE;
}

void StateSupportDeath::Exit()
{
}
#pragma endregion

#pragma region healing state
StateSupportHealing::StateSupportHealing(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateSupportHealing::~StateSupportHealing()
{
}

void StateSupportHealing::Enter()
{
	m_go->moveSpeed = 1;
	m_go->actionSpeed = 0.2;
}

void StateSupportHealing::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
		m_go->sm->m_currState->Exit();
		m_go->sm->m_currState = m_go->sm->m_nextState;
		m_go->sm->m_currState->Enter();
		return;
	}
	if (m_go->alliesActiveCount == 0)
	{
		m_go->sm->SetNextState("Hiding");
		return;
	}
	if (m_go->health < 40)
	{
		m_go->sm->SetNextState("Hurt");
		return;
	}
	if (m_go->urgent)
	{
		m_go->sm->SetNextState("UrgentHealing");
		return;
	}
	if (m_go->nearest->sm->GetCurrentState() == "Healthy") {
		m_go->sm->SetNextState("Healthy");
		m_go->healTarget = NULL;
		return;
	}
	if (m_go->healTarget == NULL || m_go->healTarget->active == false)
	{
		m_go->sm->SetNextState("Healthy");
		m_go->healTarget = NULL;
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

	if (m_go->nearest) {
		if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize * 3)
		{
			m_go->moving = false;
			if (m_go->EnergyReduce(m_go->actionSpeed))
			{
				m_go->nearest->health += 4;
			}
		}
		else {
			m_go->moving = true;
		}
	}
}
void StateSupportHealing::Exit()
{
}
#pragma endregion

#pragma region urgent healing state
StateSupportUrgentHealing::StateSupportUrgentHealing(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateSupportUrgentHealing::~StateSupportUrgentHealing()
{
}

void StateSupportUrgentHealing::Enter()
{
	m_go->moveSpeed = 2;
	m_go->actionSpeed = 0.3;
}

void StateSupportUrgentHealing::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
		m_go->sm->m_currState->Exit();
		m_go->sm->m_currState = m_go->sm->m_nextState;
		m_go->sm->m_currState->Enter();
		return;
	}
	if (m_go->alliesActiveCount == 0)
	{
		m_go->sm->SetNextState("Hiding");
		return;
	}
	if (m_go->health < 40)
	{
		m_go->sm->SetNextState("Hurt");
		m_go->urgent = false;
		return;
	}
	if (m_go->nearest->sm->GetCurrentState() != "NearDeath")
	{
		m_go->urgent = false;
		m_go->sm->SetNextState("Healing");
		return;
	}
	if (m_go->healTarget == NULL || m_go->healTarget->active == false)
	{
		m_go->sm->SetNextState("Healthy");
		m_go->urgent = false;
		m_go->healTarget = NULL;
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

	if (m_go->nearest) {
		if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize * 3)
		{
			m_go->moving = false;
			if (m_go->EnergyReduce(m_go->actionSpeed))
			{
				m_go->nearest->health += 8;
			}
		}
		else {
			m_go->moving = true;
		}
	}
}
void StateSupportUrgentHealing::Exit()
{
}
#pragma endregion

#pragma region hurt state
StateSupportHurt::StateSupportHurt(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateSupportHurt::~StateSupportHurt()
{
}

void StateSupportHurt::Enter()
{
	m_go->moveSpeed = 0.8;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateSupportHurt::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
		m_go->sm->m_currState->Exit();
		m_go->sm->m_currState = m_go->sm->m_nextState;
		m_go->sm->m_currState->Enter();
		return;
	}
	if (m_go->alliesActiveCount == 0)
	{
		m_go->sm->SetNextState("Hiding");
		return;
	}
	if (m_go->health > 80) {
		m_go->sm->SetNextState("Healthy");
		m_go->hiding = false;
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
	if (m_go->nearest != NULL) {
		if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize) {
			m_go->moving = false;
			if (m_go->EnergyReduce(m_go->actionSpeed))
			{
				m_go->health += 5;
				m_go->hiding = true;
			}
		}
		else { m_go->moving = true;
		m_go->hiding = false;
		/*std::cout << "following" << std::endl*/; }
	}
	else { m_go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
}

void StateSupportHurt::Exit()
{
}
#pragma endregion

#pragma region hiding state
StateSupportHiding::StateSupportHiding(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateSupportHiding::~StateSupportHiding()
{
}

void StateSupportHiding::Enter()
{
	m_go->moveSpeed = 0.6;
	m_go->actionSpeed = 0.2;
	m_go->target = m_go->pos;
	m_go->nearest = NULL;
}

void StateSupportHiding::Update(double dt)
{
	if (m_go->health <= 0)
	{
		m_go->sm->SetNextState("Death");
		m_go->sm->m_currState->Exit();
		m_go->sm->m_currState = m_go->sm->m_nextState;
		m_go->sm->m_currState->Enter();
		return;
	}
	if (m_go->alliesActiveCount > 0)
	{
		m_go->sm->SetNextState("Healthy");
		m_go->hiding = false;
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
	if (m_go->nearest != NULL) {
		if ((m_go->nearest->pos - m_go->pos).Length() < m_gridSize) {
			m_go->moving = false;
			if (m_go->EnergyReduce(m_go->actionSpeed))
			{
				m_go->health += 5;
				m_go->hiding = true;
			}
		}
		else {
			m_go->moving = true;
			m_go->hiding = false;
			/*std::cout << "following" << std::endl*/;
		}
	}
	else { m_go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
}

void StateSupportHiding::Exit()
{
}
#pragma endregion






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
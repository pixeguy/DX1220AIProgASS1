#include "StatesMechanic.h"
#include "SceneData.h"
#include "GridSettings.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

#pragma region healthy state
StateMechanicHealthy::StateMechanicHealthy(const std::string & stateID, GameObject * go)
	: State(stateID),
	go(go)
{
}

StateMechanicHealthy::~StateMechanicHealthy()
{
}

void StateMechanicHealthy::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->nearest = NULL;
	go->actionSpeed = 0.06;
}

void StateMechanicHealthy::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->specialTarget != NULL && go->specialTarget->active == true)
	{
		go->nearest = go->specialTarget;
		go->sm->SetNextState("GoldenChase");
		go->sm->m_currState->Exit();
		go->sm->m_currState = go->sm->m_nextState;
		go->sm->m_currState->Enter();
		return;
	}
	if (go->steps >= 3)
	{
		go->sm->SetNextState("Hiding");
		return;
	}
	if (go->health <= 40)
	{
		go->sm->SetNextState("Hurt");
		return;
	}
	//go->sm->SetNextState("Building");
	if(go->healTarget != NULL && go->healTarget->active == true)
	{
		go->sm->SetNextState("Healing");
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

	//if close enough to a spawner
	float distance = 999;
	if(go->nearest != NULL)
	{ 
		distance = (go->pos - go->nearest->pos).Length();
	}
	if (distance < 5) //grid size is 5
	{
		GameObject* go2 = go->nearest;
		go->moving = false;
		if (go->EnergyReduce(go->actionSpeed))
		{
			if (go->choice != 0)
			{
				if (go->choice == 1)
				{
					go->nearest->woodenLogs++;
				}
				else if (go->choice == 2)
				{
					go->nearest->metalParts++;
				}
			}
		}
	}
	else {
		go->moving = true;
	}
}

void StateMechanicHealthy::Exit()
{
}
#pragma endregion

#pragma region healing state
StateMechanicHealing::StateMechanicHealing(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicHealing::~StateMechanicHealing()
{
}

void StateMechanicHealing::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->actionSpeed = 0.12;
	//go->nearest = NULL;
}

void StateMechanicHealing::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->specialTarget != NULL && go->specialTarget->active == true)
	{
		go->nearest = go->specialTarget;
		go->sm->SetNextState("GoldenChase");
		return;
	}
	if (go->steps >= 3)
	{
		go->sm->SetNextState("Hiding");
		return;
	}
	if (go->health <= 40)
	{
		go->sm->SetNextState("Hurt");
		return;
	}
	if(go->nearest->health > 60)
	{
		go->sm->SetNextState("Healthy");
		go->healTarget = NULL;
		return;
	}
	if(go->healTarget == NULL || go->healTarget->active == false)
	{
		go->sm->SetNextState("Healthy");
		go->healTarget = NULL;
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

	if (go->nearest) {
		if ((go->nearest->pos - go->pos).Length() < m_gridSize * 3)
		{
			go->moving = false;
			if (go->EnergyReduce(go->actionSpeed))
			{
				go->nearest->health += 2;
			}
		}
		else {
			go->moving = true;
		}
	}
}

void StateMechanicHealing::Exit()
{
}
#pragma endregion

#pragma region Building state
StateMechanicBuilding::StateMechanicBuilding(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicBuilding::~StateMechanicBuilding()
{
}

void StateMechanicBuilding::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->nearest = NULL;
	go->energy = 0;
	go->normalTarget = Vector3(0, 0, 0);
}

void StateMechanicBuilding::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->specialTarget != NULL && go->specialTarget->active == true)
	{
		go->nearest = go->specialTarget;
		go->sm->SetNextState("GoldenChase");
		return;
	}
	if (go->steps >= 3)
	{
		go->sm->SetNextState("Hiding");
		return;
	}
	if (go->health <= 40)
	{
		go->sm->SetNextState("Hurt");
		return;
	}
	go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
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

	float distance = 999;
	distance = (go->pos - go->normalTarget).Length();
	
	if (distance < m_gridSize) //grid size is 5
	{
		go->moving = false;
		if (go->EnergyReduce(0.05))
		{
			PostOffice::GetInstance()->Send("Scene", new MessageSpawnMortar(go));
			go->sm->SetNextState("Healthy");
		}
	}
	else {
		go->moving = true;
	}
}

void StateMechanicBuilding::Exit()
{
}
#pragma endregion

#pragma region Hurt state
StateMechanicHurt::StateMechanicHurt(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicHurt::~StateMechanicHurt()
{
}

void StateMechanicHurt::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->nearest = NULL;
	go->energy = 0;
	go->normalTarget = Vector3(0, 0, 0);
	go->actionSpeed = 0.06f;
}

void StateMechanicHurt::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->specialTarget != NULL && go->specialTarget->active == true)
	{
		go->nearest = go->specialTarget;
		go->sm->SetNextState("GoldenChase");
		return;
	}
	if(go->steps >= 3)
	{
		go->sm->SetNextState("Hiding");
		return;
	}
	if (go->nearest) {
		go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
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

	//if close enough to a spawner
	float distance = 999;
	if (go->nearest != NULL)
	{
		distance = (go->pos - go->nearest->pos).Length();
	}
	if (distance < 5) //grid size is 5
	{
		GameObject* go2 = go->nearest;
		go->moving = false;
		if (go->EnergyReduce(go->actionSpeed))
		{
			if (go->choice != 0)
			{
				if (go->choice == 1)
				{
					go->nearest->woodenLogs++;
				}
				else if (go->choice == 2)
				{
					go->nearest->metalParts++;
				}
			}
		}
	}
	else { go->moving = true;
	}
}

void StateMechanicHurt::Exit()
{
}
#pragma endregion

#pragma region hiding state
StateMechanicHiding::StateMechanicHiding(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicHiding::~StateMechanicHiding()
{
}

void StateMechanicHiding::Enter()
{
	go->moveSpeed = 1.15;
	go->target = go->pos;
	go->nearest = NULL;
	go->energy = 0;
	go->normalTarget = Vector3(0, 0, 0);
}

void StateMechanicHiding::Update(double dt)
{
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->nearest) {
		go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
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

	if (go->nearest != NULL) {
		if ((go->nearest->pos - go->pos).Length() < m_gridSize) {
			go->moving = false;
			if (go->EnergyReduce(go->actionSpeed))
			{
				go->health += 5;
				go->hiding = true;
			}
		}
		else {
			go->moving = true;
			go->hiding = false;
			/*std::cout << "following" << std::endl*/;
		}
	}
	else { go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
}

void StateMechanicHiding::Exit()
{
}
#pragma endregion

#pragma region golden chase state
StateMechanicGoldenChase::StateMechanicGoldenChase(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicGoldenChase::~StateMechanicGoldenChase()
{
}

void StateMechanicGoldenChase::Enter()
{
	go->moveSpeed = 1;
	go->target = go->pos;
	go->energy = 0;
	go->normalTarget = Vector3(0, 0, 0);
	go->actionSpeed = 0.15;
}

void StateMechanicGoldenChase::Update(double dt)
{
	if (go->specialTarget == NULL || go->specialTarget->active == false)
	{
		go->sm->SetNextState("Healthy");
		return;
	}
	if (go->health <= 0)
	{
		go->sm->SetNextState("Death");
		return;
	}
	if (go->nearest) {
		go->moveLeft = go->moveRight = go->moveUp = go->moveDown = false;
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

	if (go->nearest != NULL) {
		if ((go->nearest->pos - go->pos).Length() < m_gridSize) {
			go->moving = false;
			if (go->EnergyReduce(go->actionSpeed))
			{
				if (chasingPhase) {
					go->nearest->health -= 5;
					go->nearest = go->external2;
				}
				if (!chasingPhase)
				{
					if (go->choice != 0)
					{
						if (go->choice == 1)
						{
							go->nearest->woodenLogs++;
						}
						else if (go->choice == 2)
						{
							go->nearest->metalParts++;
						}
					}
					go->nearest = go->specialTarget;
				}
				chasingPhase = !chasingPhase;
			}
		}
		else {
			go->moving = true;
		}
	}
	else { go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
}

void StateMechanicGoldenChase::Exit()
{
}
#pragma endregion

#pragma region death state
StateMechanicDeath::StateMechanicDeath(const std::string& stateID, GameObject* go)
	: State(stateID),
	go(go)
{
}

StateMechanicDeath::~StateMechanicDeath()
{
}

void StateMechanicDeath::Enter()
{
	go->moveSpeed = 5;
	go->target = go->pos;
	go->energy = 0;
	go->normalTarget = Vector3(0, 0, 0);
	go->actionSpeed = 0.4;
}

void StateMechanicDeath::Update(double dt)
{
	go->moveSpeed = 0;
	go->actionSpeed = 0.3;
	go->target = go->pos;
	go->nearest = NULL;
	go->active = false;
	go->type = GameObject::GO_NONE;
}

void StateMechanicDeath::Exit()
{
}
#pragma endregion
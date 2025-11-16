#include "SceneMovement_Week03.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesFish.h"
#include "StatesShark.h"
#include "StatesBuilding.h"
#include "StatesMechanic.h"
#include "StatesRanged.h"


#include "StatesAttacker.h"
#include "StatesSupport.h"
#include "SceneData.h"

SceneMovement_Week03::SceneMovement_Week03()
{
}

SceneMovement_Week03::~SceneMovement_Week03()
{
}

void SceneMovement_Week03::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight;

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	SceneData::GetInstance()->SetObjectCount(0);
	SceneData::GetInstance()->SetFishCount(0);
	SceneData::GetInstance()->SetNumGrid(20);
	SceneData::GetInstance()->SetGridSize(m_worldHeight / SceneData::GetInstance()->GetNumGrid());
	SceneData::GetInstance()->SetGridOffset(SceneData::GetInstance()->GetGridSize() * 0.5f);
	m_noGrid = 20;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;
	m_hourOfTheDay = 0;

	//GameObject* go = FetchGO(GameObject::GO_SHARK);
	//go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
	//go->pos.Set(m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, 0);
	//go->target = go->pos;

	InitMainBase(GameObject::SIDE_BLUE, Vector3(m_worldWidth - m_gridSize, m_worldHeight / 2, 0.f));
	InitMainBase(GameObject::SIDE_RED, Vector3(0 + m_gridSize, m_worldHeight / 2, 0.f));
	InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, m_worldHeight / 4, 0.f));
	InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, (m_worldHeight / 4) * 3, 0.f));
	InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), m_worldHeight / 4, 0.f));
	//InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4)  + 10, m_worldHeight / 4, 0.f));
	InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), (m_worldHeight / 4) * 3, 0.f));

	PostOffice::GetInstance()->Register("Scene", this);

	srand(time(NULL));
}

GameObject* SceneMovement_Week03::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active && go->type == type)
		{
			go->active = true;
			return go;
		}
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		GameObject* go = new GameObject(type);
		m_goList.push_back(go);


		if (type == GameObject::GO_FISH)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateTooFull("TooFull", go));
			go->sm->AddState(new StateFull("Full", go));
			go->sm->AddState(new StateHungry("Hungry", go));
			go->sm->AddState(new StateDead("Dead", go));
		}
		else if (type == GameObject::GO_SHARK)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateCrazy("Crazy", go));
			go->sm->AddState(new StateNaughty("Naughty", go));
			go->sm->AddState(new StateHappy("Happy", go));
		}
		else if (type == GameObject::GO_MAINBASE)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateBuildingHealthy("Healthy", go));
		}
		else if (type == GameObject::GO_SPAWNER)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateBuildingHealthy("Healthy", go));
		}
		else if (type == GameObject::GO_MECHANIC)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateMechanicHealthy("Healthy", go));
		}
		else if (type == GameObject::GO_RANGED)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateRangedHealthy("Healthy", go));
		}
		else if (type == GameObject::GO_ATTACKER)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateAttackerHealthy("Healthy", go));
		}
		else if (type == GameObject::GO_SUPPORT)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None","Healthy", go));
			go->sm->AddState(new StateSupportHealthy("Healthy", go));
			go->sm->AddState(new StateSupportDeath("Death", go));
		}
	}
	return FetchGO(type);
}

GameObject* SceneMovement_Week03::FetchProj()
{
	for (std::vector<GameObject*>::iterator it = m_projList.begin(); it != m_projList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject* go = new GameObject(GameObject::GO_PROJECTILE);
		m_projList.push_back(go);
	}
	return FetchProj();
}


GameObject* SceneMovement_Week03::InitMainBase(GameObject::SIDE side, Vector3 pos)
{
	GameObject* mainBase = FetchGO(GameObject::GO_MAINBASE);
	mainBase->pos = pos;
	mainBase->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize * 2);
	mainBase->side = side;
	mainBase->target = mainBase->pos;
	mainBase->sm->SetNextState("Healthy");
	mainBase->moving = false;
	return mainBase;
}

GameObject* SceneMovement_Week03::InitSpawner(GameObject::SIDE side, Vector3 pos)
{
	GameObject* spawner = FetchGO(GameObject::GO_SPAWNER);
	spawner->pos = pos;
	spawner->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize);
	spawner->side = side;
	spawner->target = spawner->pos;
	spawner->energy = 0;
	spawner->maxEnergy = 100;
	spawner->maxHealth = 100;
	spawner->health = 90;
	spawner->sm->SetNextState("Healthy");
	spawner->woodenLogs = spawner->metalParts = 0;
	spawner->moving = false;
	m_spawners.push_back(spawner);
	return spawner;
}

Vector3 SceneMovement_Week03::RandomPointInRing(const Vector3& center, float minRadius, float maxRadius)
{
	float angle = Math::RandFloatMinMax(0.f, 2 * Math::PI);

	 //Choose a random radius between min and max (uniform area)
	float r = sqrt(Math::RandFloatMinMax(minRadius * minRadius, maxRadius * maxRadius));

	float x = center.x + r * cos(angle);
	float y = center.y + r * sin(angle);

	return Vector3(x, y, center.z);
}

GameObject* SceneMovement_Week03::SpawnUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type)
{
	static const float AttackerRate = 25;
	static const float RangedRate = 25;
	static const float SupportRate = 25;
	static const float MechanicRate = 25;
	float random = Math::RandFloatMinMax(0.f, 100.f);
	GameObject* unit = nullptr;
	bool useRandom = false;
	if (type == GameObject::GO_NONE)
	{
		useRandom = true;
	}
	if ((useRandom && random < AttackerRate) || type == GameObject::GO_ATTACKER)
	{
		unit = FetchGO(GameObject::GO_RANGED);
		unit->type = GameObject::GO_ATTACKER;
	}
	else if ((useRandom && random < AttackerRate + RangedRate && random > AttackerRate) || type == GameObject::GO_RANGED)
	{
		unit = FetchGO(GameObject::GO_RANGED);
		unit->type = GameObject::GO_RANGED;
	}
	else if ((useRandom && random < AttackerRate + RangedRate + SupportRate && random > AttackerRate + RangedRate) || type == GameObject::GO_SUPPORT)
	{
		unit = FetchGO(GameObject::GO_SUPPORT);
		unit->type = GameObject::GO_SUPPORT;
	}
	else if ((useRandom && random < 100 && random > AttackerRate + RangedRate + SupportRate) || type == GameObject::GO_MECHANIC)
	{
		unit = FetchGO(GameObject::GO_MECHANIC);
		unit->type = GameObject::GO_MECHANIC;
	}

	// i need to set a blank state first, in order to access the Enter() of the first actual state
	unit->sm->SetNextState("None");
	unit->maxHealth = 100;
	unit->health = 50;
	unit->maxEnergy = 10;

	unit->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
	unit->side = side;
	unit->pos = pos;
	unit->target = unit->pos;
	unit->moving = true;
	return unit;
}

GameObject* SceneMovement_Week03::SpawnMetalUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type)
{
	static const float TankRate = 50;
	static const float MortarRate = 50;
	float random = Math::RandFloatMinMax(0.f, 100.f);
	GameObject* unit = nullptr;
	bool useRandom = false;
	if (type == GameObject::GO_NONE)
	{
		useRandom = true;
	}
	if ((useRandom && random < TankRate) || type == GameObject::GO_TANK)
	{
		unit = FetchGO(GameObject::GO_TANK);
		unit->type = GameObject::GO_TANK;
	}
	else if ((useRandom && random < TankRate + MortarRate && random > TankRate) || type == GameObject::GO_MORTAR)
	{
		unit = FetchGO(GameObject::GO_MORTAR);
		unit->type = GameObject::GO_MORTAR;
	}

	unit->maxHealth = 100;
	unit->health = 50;
	unit->maxEnergy = 10;

	unit->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
	unit->side = side;
	unit->pos = pos;
	unit->target = unit->pos;
	unit->moveSpeed = 5.f;
	unit->moving = true;
	return unit;
}

void SceneMovement_Week03::Update(double dt)
{
	SceneBase::Update(dt);

	static const float BALL_SPEED = 5.f;
	static const float HOUR_SPEED = 1.f;
	//static enum MOVE_SPEED {
	//	NORMAL = 5,
	//};

	//static enum ACTION_SPEED {
	//	NORMAL = 1,
	//};

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight;

	m_gridSize = SceneData::GetInstance()->GetGridSize();
	m_gridOffset = SceneData::GetInstance()->GetGridOffset();
	m_noGrid = SceneData::GetInstance()->GetNumGrid();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}

	m_hourOfTheDay += HOUR_SPEED * static_cast<float>(dt) * m_speed;
	if (m_hourOfTheDay >= 24.f)
		m_hourOfTheDay = 0;

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
		GameObject* go = FetchGO(GameObject::GO_FISH);
		go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
		go->pos.Set(m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, 0);
		go->target = go->pos;
		go->steps = 0;
		go->energy = 10.f;
		go->nearest = NULL;
		go->sm->SetNextState("Full");
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
	static bool bVState = false;
	if (!bVState && Application::IsKeyPressed('V'))
	{
		bVState = true;
		GameObject* go = FetchGO(GameObject::GO_FISHFOOD);
		go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
		go->pos.Set(m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, 0);
		go->target = go->pos;
		go->moveSpeed = 1.f;
	}
	else if (bVState && !Application::IsKeyPressed('V'))
	{
		bVState = false;
	}
	static bool bBState = false;
	if (!bBState && Application::IsKeyPressed('B'))
	{
		bBState = true;
	}
	else if (bBState && !Application::IsKeyPressed('B'))
	{
		bBState = false;
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active)
				continue;
			if (go->type == GameObject::GO_SUPPORT)
			{
				go->pos = Vector3(0, 0, 0);
				go->target = Vector3(0, 0, 0);
			}
		}
	}
	static bool bFState = false;
	if (!bFState && Application::IsKeyPressed('F'))
	{
		bFState = true;

		Vector3 randomPos = RandomPointInRing(m_spawners[0]->pos, 3.75, 10);
		SpawnUnit(GameObject::SIDE_BLUE, randomPos,GameObject::GO_SUPPORT);
	}
	else if (bFState && !Application::IsKeyPressed('F'))
	{
		bFState = false;
	}
	static bool bGState = false;
	if (!bGState && Application::IsKeyPressed('G'))
	{
		bGState = true;

		Vector3 randomPos = RandomPointInRing(m_spawners[0]->pos, 3.75, 10);
		SpawnUnit(GameObject::SIDE_BLUE, randomPos, GameObject::GO_RANGED);
	}
	else if (bGState && !Application::IsKeyPressed('G'))
	{
		bGState = false;
	}

	//StateMachine
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
		if (go->sm)
			go->sm->Update(dt);
	}

	//External triggers
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
 		if (go->type == GameObject::GO_FISH)
		{
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_SHARK)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						go->energy = -1;
					}

				}
				else if (go2->type == GameObject::GO_FISHFOOD)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						go->energy += 2.5f;
						go2->active = false;
					}

				}
				if (go->sm->GetCurrentState() == "Hungry")
				{
					MessageWRU msgCheckFish = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FISHFOOD, 50.0f);
					Handle(&msgCheckFish);
				}
				else if (go->sm->GetCurrentState() == "Full")
				{
					MessageWRU msgCheckFish = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SHARK, 50.0f);
					Handle(&msgCheckFish);
				}
				//if (go2->type == GameObject::GO_SHARK)
				//{
				//	float distance = (go->pos - go2->pos).Length();
				//	if (distance < m_gridSize)
				//	{
				//		go->energy = -1;
				//	}

				//}
				//else if (go2->type == GameObject::GO_FISHFOOD)
				//{
				//	float distance = (go->pos - go2->pos).Length();
				//	if (distance < m_gridSize)
				//	{
				//		go->energy += 2.5f;
				//		go2->active = false;
				//	}

				//}
			}
		}
		else if (go->type == GameObject::GO_SHARK)
		{
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			float highestEnergy = FLT_MIN;
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_FISH)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						go->energy = -1;
					}
				}
			}
			if (go->sm->GetCurrentState() == "Naughty")
			{
				MessageWRU msgCheckFish = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FULLFISH, 50.0f);
				Handle(&msgCheckFish);
			}
			else if (go->sm->GetCurrentState() == "Crazy")
			{
				MessageWRU msgCheckFish = MessageWRU(go, MessageWRU::SEARCH_TYPE::HIGHEST_ENERGYFISH, 50.0f);
				Handle(&msgCheckFish);
			}
		}
		else if (go->type == GameObject::GO_SPAWNER)
		{
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go->sm->GetCurrentState() == "Healthy")
				{

				}
			}
		}
		else if (go->type == GameObject::GO_MECHANIC)
		{
			if (go->sm->GetCurrentState() == "Healthy")
			{
				MessageWRU msgCheckSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 50.0f);
				Handle(&msgCheckSpawner);
			}
		}
		else if (go->type == GameObject::GO_RANGED)
		{
			if (go->sm->GetCurrentState() == "Healthy")
			{
				//if no target, or if current target died
				if (go->nearest == NULL || go->nearest->active == false) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
				else //use 10 - 15 for radius to check whether enemy is too close
				{
					if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
					{
						go->moving = false;
						float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
						if (go->EnergyReduce(finalActionSpeed))
						{
							GameObject* projectile = FetchProj();
							projectile->type = GameObject::GO_PROJECTILE;
							projectile->pos = go->pos;
							projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);

							Vector3 dir = (go->nearest->pos - go->pos).Normalized();
							float overshootDistance = 100.0f;
							projectile->target = go->nearest->pos + dir * overshootDistance;
							projectile->nearest = go->nearest; //use nearest as proj target object
							projectile->moveSpeed = 20.f;
							projectile->side = go->side;
						}
					}
					else
					{
						go->moving = true;
						MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
						Handle(&msgCheckEnemy);
					}
				}
			}
		}
		else if (go->type == GameObject::GO_ATTACKER)
		{
			//if no target, or if current target died
			if (go->nearest == NULL || go->nearest->active == false) {
				MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
				Handle(&msgCheckEnemy);
			}
			else //use 10 - 15 for radius to check whether enemy is too close
			{
				if ((go->nearest->pos - go->pos).Length() < m_gridSize)
				{
					go->moving = false;
					if (go->EnergyReduce(0.3f))
					{
						go->nearest->health -= 5;
					}
				}
				else { //if im not there yet, continue checking
					go->moving = true;
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
			}
		}
		else if (go->type == GameObject::GO_SUPPORT) //RMBBB  LOOK HEREREEEEEE, make it so that once u have a target healing, dont heal another guy even if they spawn closer. unless they are calling.
		{
			MessageWRU msgCheckAllyNoSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ALLY_NOSUP, 200.0f);
			Handle(&msgCheckAllyNoSup);
			if (go->nearest != NULL) {
				if ((go->nearest->pos - go->pos).Length() < m_gridSize * 10){
					go->nearest->supportSpeed = 5;
					go->nearest->supportActionSpeed = 0.5;
					go->moving = false;
					/*std::cout << "followed something to close" << std::endl;*/
				}
				else { go->moving = true; go->nearest->supportSpeed = 0; go->nearest->supportActionSpeed = 0; /*std::cout << "following" << std::endl*/; }
			}
			else { go->moving = false; /*std::cout << "cant find anything" << std::endl;*/ }
		}
	}


	//loop through all spawners
	for (std::vector<GameObject*>::iterator it = m_spawners.begin(); it != m_spawners.end(); ++it)
	{
		GameObject* spawner = *it;

		spawner->energy += 1; // dont wanna use EnergyReduce function, i want energy to stop at max
		GameObject::SIDE side = spawner->side;
		//deep nested code ahead !!! BEWARE !!! i lazy recode it lol

		//spawner uses random number if both materials have enough, to spawn random wooden or metal unit
		// if spawner doesnt have enough of both, it will use whichever is enough else no spawn at all
		// if spawner cant find a mechanic at all spawn mechanic first , if not enough wood for mechanic then just spawn metal
		if (spawner->energy >= spawner->maxEnergy)
		{
			spawner->energy = spawner->maxEnergy;
			float randomNumber = -1;
			bool spawnWoodUnit = false;
			if (spawner->woodenLogs > 2 && spawner->metalParts > 2)
			{
				randomNumber = Math::RandFloatMinMax(0.f, 100.f);
			}
			//spawn units
			if (randomNumber > -1)
			{
				if (randomNumber < 50)
				{
					spawnWoodUnit = true;
				}
				else if (randomNumber >= 50) // spawn metal units
				{
					spawnWoodUnit = false;
				}


				bool haveMechanic = false;
				for (int i = 0; i < m_goList.size(); i++)
				{
					if (m_goList[i]->type == GameObject::GO_MECHANIC)
					{
						haveMechanic = true;
						break;
					}
				}

				//if no mechanics, dont care about random, just spawn mechanic, if not enough wood for mechanic, then just spawn metal
				if (!haveMechanic) {
					if (spawner->woodenLogs > 2) {
						Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
						SpawnUnit(side, randomPos, GameObject::GO_MECHANIC);
					}
					else if (spawner->metalParts > 2) {
						Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
						SpawnMetalUnit(side, randomPos, GameObject::GO_RANGED); //spawn random unit

						spawner->energy = 0;
						spawner->metalParts -= 2;
						std::cout << "Spawned MetalUnit Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
					}
				}
				else {
					if (spawnWoodUnit)
					{
						if (spawner->woodenLogs > 2) {
							Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
							SpawnUnit(side, randomPos, GameObject::GO_RANGED);

							spawner->energy = 0;
							spawner->woodenLogs -= 2;
							std::cout << "Spawned WoodUnit Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
						}
					}
					else // spawn metal units
					{
						if (spawner->metalParts > 2) {
							Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
							SpawnMetalUnit(side, randomPos); //spawn random unit

							spawner->energy = 0;
							spawner->metalParts -= 2;
							std::cout << "Spawned MetalUnit Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
						}
					}
				}
			}
			else
			{
				bool haveMechanic = false;
				for (int i = 0; i < m_goList.size(); i++)
				{
					if (m_goList[i]->type == GameObject::GO_MECHANIC)
					{
						haveMechanic = true;
						break;
					}
				}

				if (spawner->woodenLogs > 2)
				{
					if (!haveMechanic) {
						Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
						SpawnUnit(side, randomPos, GameObject::GO_MECHANIC);
					}
					else
					{
						Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
						SpawnUnit(side, randomPos, GameObject::GO_RANGED);
					}
					spawner->energy = 0;
					spawner->woodenLogs -= 2;
					std::cout << "Spawned NotRandom WOoden Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
				}
				else if (spawner->metalParts > 2)// spawn metal units
				{
					Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10.f);
					SpawnMetalUnit(side, randomPos); //spawn random unit

					spawner->energy = 0;
					spawner->metalParts -= 2;
					std::cout << "Spawned NotRandom Metal Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
				}
				
			}
		}

		//debug spawner materials
		if (spawner->woodenLogs != 0)
		{
			//std::cout << "Logs: " << spawner->woodenLogs << "  Metal: " << spawner->metalParts << std::endl;
		}
	}


	//loop through all projectiles
	for (std::vector<GameObject*>::iterator it = m_projList.begin(); it != m_projList.end(); ++it)
	{
		GameObject* proj = *it;
		if (!proj->active)
			continue;
		MessageWRU msgCheckEnemy = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 50.0f);
		Handle(&msgCheckEnemy);
		if (proj->nearest != NULL) {
			float distance = (proj->nearest->pos - proj->pos).Length();
			if (distance <= proj->nearest->scale.x)
			{
				proj->health = 0;
				proj->type = GameObject::GO_NONE;
				proj->active = false;
				proj->nearest->health -= 5;
			}
		}
		//std::cout << proj->target << std::endl;
	}

	//check for death buildings and units
	{
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active)
				continue;
			if (go->health <= 0) {
				go->sm->SetNextState("Death");

				//go->type = GameObject::GO_NONE;
				//go->active = false;
				--m_objectCount;
			}
		}

		for (std::vector<GameObject*>::iterator it = m_spawners.begin(); it != m_spawners.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active)
				continue;
			if (go->health <= 0) {

				go->type = GameObject::GO_NONE;
				go->active = false;
				--m_objectCount;
			}
		}
	}

	//Movement Section
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
		Vector3 dir = go->target - go->pos;

		//use additive speed so i can add on whenever i want
		go->finalMoveSpeed = (go->moveSpeed * 5) + go->supportSpeed;
		if (go->type == GameObject::GO_SUPPORT) { std::cout << go->moveSpeed << std::endl; }
		if (go->moving == true) {
			if (dir.Length() < go->finalMoveSpeed * dt * m_speed)
			{
				//GO->pos reach target
				go->pos = go->target;
				float random = Math::RandFloatMinMax(0.f, 1.f);
				if (random < 0.25f && go->moveRight)
					go->target = go->pos + Vector3(m_gridSize, 0, 0);
				else if (random < 0.5f && go->moveLeft)
					go->target = go->pos + Vector3(-m_gridSize, 0, 0);
				else if (random < 0.75f && go->moveUp)
					go->target = go->pos + Vector3(0, m_gridSize, 0);
				else if (go->moveDown)
					go->target = go->pos + Vector3(0, -m_gridSize, 0);
				if (go->target.x < 0 || go->target.x > m_noGrid * m_gridSize || go->target.y < 0 || go->target.y > m_noGrid * m_gridSize)
					go->target = go->pos;
			}
			else
			{
				try
				{
					dir.Normalize();
					go->pos += dir * go->finalMoveSpeed * static_cast<float>(dt) * m_speed;
				}
				catch (DivideByZero)
				{
				}
			}
		}
	}
	for (std::vector<GameObject*>::iterator it = m_projList.begin(); it != m_projList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
		Vector3 dir = go->target - go->pos;
		if (dir.Length() < go->moveSpeed * dt * m_speed)
		{
			//GO->pos reach target
			go->pos = go->target;
		}
		else
		{
			try
			{
				dir.Normalize();
				go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
			}
			catch (DivideByZero)
			{
			}
		}
	}

	//Counting objects
	int objectCount = 0;
	m_numGO[GameObject::GO_FISH] = m_numGO[GameObject::GO_SHARK] = m_numGO[GameObject::GO_FISHFOOD] = 0;
	MessageCheckActive msgCheckActive = MessageCheckActive();
	MessageCheckFish msgCheckFish = MessageCheckFish();
	MessageCheckFood msgCheckFood = MessageCheckFood();
	MessageCheckShark msgCheckShark = MessageCheckShark();
	for (GameObject* go : m_goList)
	{
		objectCount += static_cast<int>(go->Handle(&msgCheckActive));
		m_numGO[GameObject::GO_FISH] += static_cast<int>(go->Handle(&msgCheckFish));
		m_numGO[GameObject::GO_FISHFOOD] += static_cast<int>(go->Handle(&msgCheckFood));
		m_numGO[GameObject::GO_SHARK] += static_cast<int>(go->Handle(&msgCheckShark));
	}
	SceneData::GetInstance()->SetObjectCount(objectCount);
	SceneData::GetInstance()->SetFishCount(m_numGO[GameObject::GO_FISH]);
}

void SceneMovement_Week03::RenderGOBar(GameObject* go, float vertScale)
{
	float healthRatio = go->health / go->maxHealth;
	float energyRatio = go->energy / go->maxEnergy;
	float barWidth = go->scale.x / 1.8f;
	float barHeight = go->scale.y / vertScale;

	// draw the health (shrinks left -> right)
	modelStack.PushMatrix();
	// shift left edge fixed: move half the reduced width to the left
	float offsetX = -(barWidth * (1.f - healthRatio));
	modelStack.Translate(go->pos.x + offsetX, go->pos.y - 3.5f, zOffset);
	modelStack.Scale(barWidth * healthRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_CUBE], false);
	modelStack.PopMatrix();

	// draw the background (max health)
	modelStack.PushMatrix();
	modelStack.Translate(go->pos.x, go->pos.y - 3.5f, zOffset);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXCUBE], false);
	modelStack.PopMatrix();


	modelStack.PushMatrix();
	// shift left edge fixed: move half the reduced width to the left
	float energyoffsetX = -(barWidth * (1.f - energyRatio));
	modelStack.Translate(go->pos.x + energyoffsetX, go->pos.y - 4.9, zOffset);
	modelStack.Scale(barWidth * energyRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_ENERGYCUBE], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(go->pos.x, go->pos.y - 4.9, zOffset);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXENERGYCUBE], false);
	modelStack.PopMatrix();
}

void SceneMovement_Week03::RenderGO(GameObject* go)
{
	std::ostringstream ss;
	switch (go->type)
	{
	case GameObject::GO_BALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);

		ss.str("");
		ss.precision(3);
		ss << go->id;
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISH:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->sm)
		{
			if (go->sm->GetCurrentState() == "TooFull")
				RenderMesh(meshList[GEO_TOOFULL], false);
			else if (go->sm->GetCurrentState() == "Full")
				RenderMesh(meshList[GEO_FULL], false);
			else if (go->sm->GetCurrentState() == "Hungry")
				RenderMesh(meshList[GEO_HUNGRY], false);
			else
				RenderMesh(meshList[GEO_DEAD], false);
		}

		// Exercise Week 4
		modelStack.PushMatrix();
		ss.precision(3);
		ss << "[" << go->pos.x << ", " << go->pos.y << "]";
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(-SceneData::GetInstance()->GetGridSize() / 4, SceneData::GetInstance()->GetGridSize() / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();

		// Exercise Week 4
		modelStack.PushMatrix();
		ss.str("");
		ss.precision(3);
		ss << go->energy;
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(0, -SceneData::GetInstance()->GetGridSize() / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		// Exercise Week 4
		modelStack.PopMatrix();
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHARK:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->sm)
		{
			if (go->sm->GetCurrentState() == "Crazy")
				RenderMesh(meshList[GEO_CRAZY], false);
			else if (go->sm->GetCurrentState() == "Happy")
				RenderMesh(meshList[GEO_HAPPY], false);
			else
				RenderMesh(meshList[GEO_SHARK], false);
		}
		modelStack.PushMatrix();
		ss.str("");
		ss.precision(3);
		ss << "[" << go->pos.x << ", " << go->pos.y << "]";
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(-SceneData::GetInstance()->GetGridSize() / 4, -SceneData::GetInstance()->GetGridSize() / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		// Exercise Week 4

		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISHFOOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_FISHFOOD], false);
		// Exercise Week 4
		modelStack.PushMatrix();
		ss << "[" << go->pos.x << ", " << go->pos.y << "]";
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(-SceneData::GetInstance()->GetGridSize() / 4, -SceneData::GetInstance()->GetGridSize() / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();		modelStack.PopMatrix();
		break;
	case GameObject::GO_MAINBASE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_BASERED], false);
		else
			RenderMesh(meshList[GEO_BASEBLUE], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);
		break;
	case GameObject::GO_SPAWNER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_SPAWNERRED], false);
		else
			RenderMesh(meshList[GEO_SPAWNERBLUE], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset + 0.001f); // small offset to prevent z-fighting
		modelStack.Scale(7.5 * 2, 7.5 * 2, 1); // radius (scale by 2 for diameter)
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset + 0.001f); // small offset to prevent z-fighting
		modelStack.Scale(20 * 2, 20 * 2, 1); // radius (scale by 2 for diameter)
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);
		break;
	case GameObject::GO_ATTACKER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ATTACKER], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	case GameObject::GO_RANGED:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->moveLeft)
			RenderMesh(meshList[GEO_RANGED], false);
		else
			RenderMesh(meshList[GEO_RANGEDRIGHT], false);
		modelStack.PopMatrix();

		RenderGOBar(go, 7);
		break;
	case GameObject::GO_SUPPORT:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->moveLeft)
			RenderMesh(meshList[GEO_RANGED], false);
		else
			RenderMesh(meshList[GEO_RANGEDRIGHT], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);


		if (go->nearest != NULL && go->nearest->active)
		{
			Vector3 start = go->pos;
			Vector3 end = go->nearest->pos;
			Vector3 diff = end - start;
			float distance = diff.Length();

			Vector3 dir = diff.Normalized();
			float angle = Math::RadianToDegree(atan2(dir.y, dir.x));

			modelStack.PushMatrix();
			{
				// 1️⃣ Move to start point
				modelStack.Translate(start.x, start.y, zOffset + 0.1f);

				// 2️⃣ Rotate toward the target
				modelStack.Rotate(angle, 0, 0, 1);

				// 3️⃣ Scale to the total distance
				modelStack.Scale(distance, 0.1f, 1.f);

				// 4️⃣ Move *after scaling*, so only half the cube's local X (0.5) unscaled
				modelStack.Translate(0.5f / distance, 0.0f, 0.0f);
				// 3️⃣ Scale to the total distance
				modelStack.Scale(0.5,1,1);
				// 4️⃣ Move *after scaling*, so only half the cube's local X (0.5) unscaled
				modelStack.Translate(-dir.x, -dir.y, dir.z);

				// 5️⃣ Render line
				RenderMesh(meshList[GEO_CUBE], false);
			}
			modelStack.PopMatrix();
		}


		break;
	case GameObject::GO_PROJECTILE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SUPPORT], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_MECHANIC:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->moveLeft)
			RenderMesh(meshList[GEO_RANGED], false);
		else
			RenderMesh(meshList[GEO_RANGEDRIGHT], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
	}
}


bool SceneMovement_Week03::Handle(Message* message)
{
	MessageWRU* messageWRU = dynamic_cast<MessageWRU*>(message);
	if (messageWRU)
	{
		GameObject* go = messageWRU->go;
		go->nearest = nullptr;

		float nearestDistance = FLT_MAX;
		float highestEnergy = FLT_MIN;

		for (GameObject* go2 : m_goList)
		{
			if (!go2->active)
				continue;
			if (messageWRU->type == MessageWRU::NEAREST_SPAWNER && go2->type == GameObject::GO_SPAWNER && go->side == go2->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ENEMY && go2->side != go->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ALLY_NOSUP && go2->side == go->side)
			{
				if (go2 != go) {
					if (go2->type != GameObject::GO_SPAWNER && go2->type != GameObject::GO_MAINBASE && go2->type != GameObject::GO_SUPPORT) {
						float distance = (go->pos - go2->pos).Length();
						bool alreadySupported = false;

						// Check if go2 already has a support pointing to it
						for (auto go3 : m_goList)
						{
							if (!go3->active) continue;
							if (go3->type != GameObject::GO_SUPPORT) continue;

							if (go3->nearest == go2)
							{
								alreadySupported = true;
								break;
							}
						}

						// If it's not supported yet, consider it for nearest
						if (!alreadySupported)
						{
							if (distance < messageWRU->threshold && distance < nearestDistance)
							{
								nearestDistance = distance;
								go->nearest = go2;
							}
						}
					}
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_FISHFOOD && go2->type == GameObject::GO_FISHFOOD)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_SHARK && go2->type == GameObject::GO_SHARK)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_FULLFISH && go2->type == GameObject::GO_FISH)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < nearestDistance &&
					(go2->sm->GetCurrentState() == "TooFull" || go2->sm->GetCurrentState() == "Full"))
				{
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::HIGHEST_ENERGYFISH && go2->type == GameObject::GO_FISH)
			{
				if (go2->energy > highestEnergy)
				{
					highestEnergy = go2->energy;
					go->nearest = go2;
				}
			}
		}

		return true;
	}
	return false;
}

void SceneMovement_Week03::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth * (float)Application::GetWindowWidth() / Application::GetWindowHeight(), 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_BG], false);
	modelStack.PopMatrix();

	zOffset = 0;
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active)
		{
			zOffset += 0.001f;
			RenderGO(go);
		}
	}
	for (std::vector<GameObject*>::iterator it = m_projList.begin(); it != m_projList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active)
		{
			zOffset += 0.001f;
			RenderGO(go);
		}
	}
	//On screen text

	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Count:" << SceneData::GetInstance()->GetObjectCount();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Fishes:" << m_numGO[GameObject::GO_FISH];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 18);

	ss.str("");
	ss << "Shark:" << m_numGO[GameObject::GO_SHARK];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 15);

	ss.str("");
	ss << "Food:" << m_numGO[GameObject::GO_FISHFOOD];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);

	RenderTextOnScreen(meshList[GEO_TEXT], "Aquarium", Color(0, 1, 0), 3, 50, 0);
}

void SceneMovement_Week03::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if (m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}

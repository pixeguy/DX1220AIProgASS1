#include "SceneMovement_Week999.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

float fFoodTimers = 0.0f;
static const float ENERGY_DROP_RATE = 0.2f;
static const float FULL_SPEED = 8.f;
static const float HUNGRY_SPEED = 4.f;

SceneMovement_Week999::SceneMovement_Week999()
{
}

SceneMovement_Week999::~SceneMovement_Week999()
{
}

void SceneMovement_Week999::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight;

	//Physics code here
	m_speed = 1.f;
	
	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 20;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;
	m_hourOfTheDay = 0;

	// Exercise: Spawn a single GO_SHARK at the start of the scene (make it fast)
	//GameObject* newShark = FetchGO();
	//newShark->type = GameObject::GO_SHARK;
	//newShark->pos = Vector3(Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset,
	//	Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset, 0.f);
	//newShark->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);	
	//newShark->moveSpeed = FULL_SPEED * 1.5f;

	//init objs
	InitMainBase(GameObject::SIDE_BLUE,Vector3(m_worldWidth - m_gridSize, m_worldHeight / 2, 0.f));
	InitMainBase(GameObject::SIDE_RED, Vector3(0 + m_gridSize, m_worldHeight / 2, 0.f));
	InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, m_worldHeight / 4, 0.f));
	InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, (m_worldHeight / 4) * 3, 0.f));
	InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), m_worldHeight / 4, 0.f));
	InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), (m_worldHeight / 4) * 3, 0.f));

}

GameObject* SceneMovement_Week999::FetchGO()
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_FISH);
		m_goList.push_back(go);
	}
	return FetchGO();
}

GameObject* SceneMovement_Week999::FetchProj()
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
		GameObject* go = new GameObject(GameObject::GO_FISH);
		m_projList.push_back(go);
	}
	return FetchProj();
}

GameObject* SceneMovement_Week999::InitMainBase(GameObject::SIDE side, Vector3 pos)
{
	GameObject* mainBase = FetchGO();
	mainBase->type = GameObject::GO_MAINBASE;
	mainBase->pos = pos;
	mainBase->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize * 2);
	mainBase->side = side;
	mainBase->target = mainBase->pos;
	return mainBase;
}

GameObject* SceneMovement_Week999::InitSpawner(GameObject::SIDE side, Vector3 pos)
{
	GameObject* spawner = FetchGO();
	spawner->type = GameObject::GO_SPAWNER;
	spawner->pos = pos;
	std::cout << m_gridSize << std::endl;
	spawner->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize);
	spawner->side = side;
	spawner->target = spawner->pos;
	spawner->energy = 0;
	spawner->maxHealth = 100;
	spawner->health = 90;
	m_spawners.push_back(spawner);
	return spawner;
}

Vector3 RandomPointInRing(const Vector3& center, float minRadius, float maxRadius)
{
	float angle = Math::RandFloatMinMax(0.f, 2 * Math::PI);

	// Choose a random radius between min and max (uniform area)
	float r = sqrt(Math::RandFloatMinMax(minRadius * minRadius, maxRadius * maxRadius));

	float x = center.x + r * cos(angle);
	float y = center.y + r * sin(angle);

	return Vector3(x, y, center.z);
}

GameObject* SceneMovement_Week999::SpawnUnit(GameObject::SIDE side, Vector3 pos)
{
	static const float AttackerRate = 33.3f;
	static const float RangedRate = 33.3f;
	static const float SupportRate = 33.4f;
	float random = Math::RandFloatMinMax(0.f, 100.f);
	GameObject* unit = FetchGO();
	unit->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
	unit->side = side;
	unit->pos = pos;
	unit->target = unit->pos;
	unit->moveSpeed = 5.f;
	//if (random < AttackerRate)
	//{
	//	unit->type = GameObject::GO_ATTACKER;
	//	unit->maxHealth = 100;
	//	unit->health = 50;
	//	unit->maxEnergy = 10;
	//}
	//else if (random < AttackerRate + RangedRate && random > AttackerRate)
	{
		unit->type = GameObject::GO_RANGED;
		unit->maxHealth = 100;
		unit->health = 50;
		unit->maxEnergy = 10;
	}
	//else
	//{
	//	unit->type = GameObject::GO_SUPPORT;
	//	unit->maxHealth = 100;
	//	unit->health = 50;
	//	unit->maxEnergy = 10;
	//}
	return unit;
}

void SceneMovement_Week999::Update(double dt)
{
	SceneBase::Update(dt);

	static const float BALL_SPEED = 5.f;
	static const float HOUR_SPEED = 1.f;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight;
	
	if(Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}

	m_hourOfTheDay += HOUR_SPEED * static_cast<float>(dt) * m_speed;
	if (m_hourOfTheDay >= 24.f)
		m_hourOfTheDay = 0;

	//Input Section
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		// Exercise: Use VK_SPACE to spawn GO_FISH in random location 
		bSpaceState = true;
		//GameObject *go = FetchGO();
		//go->type = GameObject::GO_FISH;
		//go->pos = Vector3(Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset,
		//	Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset, 0.f);
		//go->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
		//go->steps = 5000001;
		//go->currState = GameObject::STATE_FULL;	
		//go->energy = 8.f;
		//go->nearest = NULL;
		Vector3 randomPos = RandomPointInRing(m_spawners[0]->pos, 3.75,10);
		SpawnUnit(GameObject::SIDE_BLUE, randomPos);
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
	static bool bVState = false;
	if ((!bVState && Application::IsKeyPressed('R')) || (fFoodTimers > 20.0f))
	{
		// Exercise: Use another button to spawn GO_FISHFOOD in random location (make it slow) 
		bVState = true;
		GameObject *go = FetchGO();
		go->type = GameObject::GO_FISHFOOD;
		go->pos = Vector3(Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset,
			Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize + m_gridOffset, 0.f);
		go->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
		go->moveSpeed = 1.f;
		fFoodTimers = 0;
	}
	else if (bVState && !Application::IsKeyPressed('R'))
	{
		bVState = false;
	}
	fFoodTimers += (float)dt;

	//StateMachine
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		if(go->type == GameObject::GO_FISH)
		{
			//if the fish has to move in random direction
			float random = Math::RandFloatMinMax(0.0f, 1.0f);
			if (go->steps > 15) {
				if (random < 0.5f)
				{
					// Left
					go->target.x = m_worldWidth- m_gridOffset;
					go->target.y = m_gridOffset;
					go->steps = 0;
					go->prevPos = go->pos;
				}
				else //if (random < 0.5f)
				{
					// Right
					go->target.x = m_gridOffset;
					go->target.y = m_gridOffset;
					go->steps = 0;
					go->prevPos = go->pos;
				}
				//else if (random < 0.75f)
				//{
				//	// Up
				//	go->target.x = m_worldHeight - m_gridOffset;
				//	go->target.y = m_gridOffset;
				//	go->steps = 0;
				//	go->prevPos = go->pos;
				//}
				//else
				//{
				//	// Down
				//	go->target.x = m_worldHeight - m_gridOffset;
				//	go->target.y = m_worldHeight - m_gridOffset;
				//	go->steps = 0;
				//	go->prevPos = go->pos;
				//}
			}
			std::cout << go->target.x << "," << go->target.y << std::endl;
			Vector3 displacement = go->pos - go->prevPos;
			float distanceMoved = displacement.Length();

			go->steps = distanceMoved;
			std::cout << "Steps moved: " << go->steps << std::endl;
			// Exercise: Internal triggers. Implement this State Table for GO_FISH in SceneMovement::Update with a switch statement
			switch (go->currState)
			{
			case GameObject::STATE_TOOFULL:

				go->moveSpeed = 0;
				go->energy -= ENERGY_DROP_RATE * 0.3;
				if (go->energy < 10)
				{
					go->currState = GameObject::STATE_FULL;
				}

				break;
			case GameObject::STATE_FULL:

				go->moveSpeed = FULL_SPEED;
				go->energy -= ENERGY_DROP_RATE * 0;
				//implement avoid shark later
				
				if(go->energy < 5)
				{
					go->currState = GameObject::STATE_HUNGRY;
				}
				else if (go->energy >= 10)
				{
					go->currState = GameObject::STATE_TOOFULL;
				}

				break;
			case GameObject::STATE_HUNGRY:

				go->moveSpeed = HUNGRY_SPEED;
				go->energy -= ENERGY_DROP_RATE * 0;
				//implement seek food later

				if (go->energy <= 0)
				{
					go->currState = GameObject::STATE_DEAD;
					go->countDown = 3;
				}
				else if (go->energy >= 5)
				{
					go->currState = GameObject::STATE_FULL;
				}

				break;
			case GameObject::STATE_DEAD:

				go->moveSpeed = 0;
				if (go->countDown <= 0)
				{
					go->type = GameObject::GO_NONE;
					go->active = false;
					--m_objectCount;
				}
				break;
			}
			go->countDown -= static_cast<float>(dt) * 1;
		}

		if (go->type == GameObject::GO_SUPPORT)
		{
		}

		if (go->type == GameObject::GO_SPAWNER)
		{
			if (go->energy > 10)
			{
				Vector3 randomPos = RandomPointInRing(go->pos, 3.75 ,10.f);
				SpawnUnit(GameObject::SIDE_BLUE, randomPos);
				go->energy = 0;
			}
		}
	}

	//External triggers
	static const float SHARK_DIST = 10.f * m_gridSize;
	static const float FOOD_DIST = 20.f * m_gridSize;
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		if (go->type == GameObject::GO_FISH)
		{
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			for (std::vector<GameObject *>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject *go2 = (GameObject *)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_SHARK)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						// Exercise: External triggers. Any time a GO_FISH collides with GO_SHARK, set energy to -1
						go->energy = -1;
					}
					else if (distance < SHARK_DIST && distance < nearestDistance && go->currState == GameObject::STATE_FULL)
					{
						nearestDistance = distance;
						go->nearest = go2;
					}
				}
				else if (go2->type == GameObject::GO_FISHFOOD)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						//add energy to fish
						go->energy += 3.f;
						//remove fish food
						go2->type = GameObject::GO_NONE;
						go2->active = false;
						--m_objectCount;
					}
					else if (distance < FOOD_DIST && distance < nearestDistance && go->currState == GameObject::STATE_HUNGRY)
					{
						nearestDistance = distance;
						go->nearest = go2;
					}
				}
			}
		}
		if (go->type == GameObject::GO_SUPPORT)
		{
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_SPAWNER)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < m_gridSize)
					{
						go->target = go->pos;
						go2->energy += 1.f * static_cast<float>(dt) * m_speed;
						std::cout << "Spawner Energy: " << go2->energy << std::endl;
					}
					else if (distance < SHARK_DIST && distance < nearestDistance)
					{
						nearestDistance = distance;
						go->nearest = go2;
						go->target = go->nearest->pos;
					}
				}
			}
		}
		if (go->type == GameObject::GO_ATTACKER)
		{
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if(go2->side == go->side)
					continue;
				if (go2->type == GameObject::GO_SPAWNER)
				{
					float distance = (go2->pos - go->pos).Length();
					if (distance < m_gridSize)
					{
						if(go->EnergyReduce(1.f))
						{
							go2->health -= 10;
						}
						if (go2->health <= 0)
						{
							go2->type = GameObject::GO_NONE;
							go2->active = false;
							--m_objectCount;
						}
					}
					else if (distance < nearestDistance)
					{
						nearestDistance = distance;
						go->nearest = go2;
						go->target = go->nearest->pos;
					}
				}
			}
		}
		if (go->type == GameObject::GO_RANGED)
		{
			std::cout << go->nearest << std::endl;
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go2->side == go->side)
					continue;
				if (go2->type == GameObject::GO_SPAWNER)
				{
					float distance = (go2->pos - go->pos).Length();
					float diff = nearestDistance - distance;
					if (distance < m_gridSize * 5)
					{
						if (go->EnergyReduce(1.f))
						{
							GameObject* projectile = FetchProj();
							projectile->type = GameObject::GO_PROJECTILE;
							projectile->pos = go->pos;
							projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);
							projectile->target = go2->pos;
							projectile->moveSpeed = 20.f;
							projectile->side = go->side;
						}
						if (go2->health <= 0)
						{
							go2->type = GameObject::GO_NONE;
							go2->active = false;
							--m_objectCount;
						}
					}
					else if (diff > m_gridSize && distance < nearestDistance)
					{
						nearestDistance = distance;
						go->nearest = go2;
						go->target = go->nearest->pos;
					}
				}
			}
		}
	}
	//Movement Section
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
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
	m_numGO[GameObject::GO_FISH] = m_numGO[GameObject::GO_SHARK] = m_numGO[GameObject::GO_FISHFOOD] = 0;
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		++m_numGO[go->type];
	}
}

void SceneMovement_Week999::RenderGOBar(GameObject* go, float vertScale)
{
	float healthRatio = go->health / go->maxHealth;
	float energyRatio = go->energy / go->maxEnergy;
	float barWidth = go->scale.x / 1.8f;
	float barHeight = go->scale.y / vertScale;

	// draw the health (shrinks left → right)
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


void SceneMovement_Week999::RenderGO(GameObject *go)
{
	std::ostringstream ss;
	switch(go->type)
	{
	case GameObject::GO_BALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);

		ss << go->id;
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISH:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if(go->currState == GameObject::STATE_TOOFULL)
			RenderMesh(meshList[GEO_TOOFULL], false);
		else if (go->currState == GameObject::STATE_FULL)
			RenderMesh(meshList[GEO_FULL], false);
		else if (go->currState == GameObject::STATE_HUNGRY)
			RenderMesh(meshList[GEO_HUNGRY], false);
		else if (go->currState == GameObject::STATE_DEAD)
			RenderMesh(meshList[GEO_DEAD], false);

		ss.precision(3);
		ss << go->energy;
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(0, -m_gridSize / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHARK:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SHARK], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISHFOOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_FISHFOOD], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_MAINBASE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if(go->side == GameObject::SIDE_RED)
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
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_RANGED], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	case GameObject::GO_SUPPORT:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SUPPORT], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	case GameObject::GO_PROJECTILE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SUPPORT], false);
		modelStack.PopMatrix();
	}
}

void SceneMovement_Week999::Render()
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
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
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
	ss << "Count:" << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	// Exercise: Show statistics - How many fishes are in the scene ?
	ss.str("");
	ss << "Fishes:" << m_numGO[GameObject::GO_FISH];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);

	// Exercise: Show statistics - How many fish food is in the scene ?
	ss.str("");
	ss << "Fish Food:" << m_numGO[GameObject::GO_FISHFOOD];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 15);

	// Exercise: Show statistics - How many sharks are in the scene ?
	ss.str("");
	ss << "Sharks:" << m_numGO[GameObject::GO_SHARK];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 18);

	// Optional Exercise: Show statistics - How many fishes died by hunger ?
	// Optional Exercise: Show statistics - How many fishes eaten by shark ?
	// Optional Exercise: Show statistics - How many fishes overeat ?

	RenderTextOnScreen(meshList[GEO_TEXT], "Aquarium", Color(0, 1, 0), 3, 50, 0);
}

void SceneMovement_Week999::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}

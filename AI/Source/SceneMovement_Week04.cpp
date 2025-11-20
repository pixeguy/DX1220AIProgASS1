#include "SceneMovement_Week04.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesFish.h"
#include "StatesShark.h"
#include "StatesFishFood.h"
#include "SceneData.h"
// Exercise Week 04

SceneMovement_Week04::SceneMovement_Week04()
{
}

SceneMovement_Week04::~SceneMovement_Week04()
{
}

void SceneMovement_Week04::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	SceneData::GetInstance()->SetObjectCount(0);
	SceneData::GetInstance()->SetFishCount(0);
	SceneData::GetInstance()->SetNumGrid(20);
	SceneData::GetInstance()->SetGridSize(m_worldHeight / SceneData::GetInstance()->GetNumGrid());
	SceneData::GetInstance()->SetGridOffset(SceneData::GetInstance()->GetGridSize() * 0.5f);
	m_noGrid = SceneData::GetInstance()->GetNumGrid();
	m_gridSize = SceneData::GetInstance()->GetGridSize();
	m_gridOffset = SceneData::GetInstance()->GetGridOffset();
	// Exercise Week 4
	// In SceneMovement_Week04::Init(), add these codes to define the dimensions of the grids. 

	// Also change the last 3 lines of codes to get the values from SceneData instance.

	m_hourOfTheDay = 0;

	GameObject* go = FetchGO(GameObject::GO_SHARK);
	go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
	go->pos.Set(m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, 0);
	go->target = go->pos;

	// Exercise Week 4
	// Register this scene with the "post office"
	// Post office will now be capable of addressing this scene with messages
	PostOffice::GetInstance()->Register("Scene", this);
	
	srand(time(NULL));
}

GameObject* SceneMovement_Week04::FetchGO(GameObject::GAMEOBJECT_TYPE type)
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
		else if (type == GameObject::GO_FISHFOOD)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateGrow("Grow", go));
			go->sm->AddState(new StateEvolve("Evolve", go));
		}
	}
	return FetchGO(type);
}

void SceneMovement_Week04::Update(double dt)
{
	SceneBase::Update(dt);

	static const float HOUR_SPEED = 1.f;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Exercise Week 04
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
		GameObject* go = FetchGO(GameObject::GO_SHARK);
		go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
		go->pos.Set(m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, m_gridOffset + Math::RandIntMinMax(0, m_noGrid - 1) * m_gridSize, 0);
		go->target = go->pos;
	}
	else if (bBState && !Application::IsKeyPressed('B'))
	{
		bBState = false;
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
		else if (go->type == GameObject::GO_FISHFOOD)
		{
			std::cout << go->sm->GetCurrentState()  << " , "  << go->countDown << std::endl;
		}
	}

	//Movement Section
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
		Vector3 dir = go->target - go->pos;
		if (dir.Length() < go->moveSpeed * dt * m_speed)
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
				go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
			}
			catch (DivideByZero)
			{
			}
		}
	}

	// Exercise Week 4
	// Counting objects and fishes
	int objectCount = 0;
	//int fishCount = 0;
	//m_numGO[GameObject::GO_FISH] = m_numGO[GameObject::GO_SHARK] = m_numGO[GameObject::GO_FISHFOOD] = 0;
	//for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	//{
	//	GameObject* go = (GameObject*)*it;
	//	if (!go->active)
	//		continue;
	//	++objectCount;
	//	++m_numGO[go->type];
	//	if (go->type == GameObject::GO_FISH)
	//		++fishCount;
	//}

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


void SceneMovement_Week04::RenderGO(GameObject* go)
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

		{
			modelStack.PushMatrix();
			const Vector3 displacement = go->target - go->pos;
			modelStack.Rotate(Math::RadianToDegree(atan2(displacement.y, displacement.x)), 0, 0, 1);
			modelStack.Scale(displacement.Length() / SceneData::GetInstance()->GetGridSize(), .3f, 1.f);
			RenderMesh(meshList[GEO_LINE], false);
			modelStack.PopMatrix();
		}
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
	}
}

bool SceneMovement_Week04::Handle(Message* message)
{
	MessageSpawnFood* msgSpawnFood = dynamic_cast<MessageSpawnFood*>(message);
	if (msgSpawnFood) {
		for (int i = 0; i < msgSpawnFood->count; i++)
		{
			GameObject* go = new GameObject((GameObject::GAMEOBJECT_TYPE)(msgSpawnFood->type));
			go->scale.Set(m_gridSize, m_gridSize, m_gridSize);
			go->pos.Set(m_gridOffset + msgSpawnFood->go->pos.x +
				Math::RandIntMinMax(msgSpawnFood->distRange[0], msgSpawnFood->distRange[1]) * m_gridSize,
				m_gridOffset + msgSpawnFood->go->pos.y +
				Math::RandIntMinMax(msgSpawnFood->distRange[0], msgSpawnFood->distRange[1]) * m_gridSize, 0);
			go->target = go->pos;
			go->moveSpeed = 1;
			go->active = true;
			go->sm = new StateMachine();
			go->sm->AddState(new StateGrow("Grow", go));
			go->sm->AddState(new StateEvolve("Evolve", go));
			go->sm->SetNextState("Grow");
			m_goList.push_back(go);
		}
	}

	MessageStop* msgFishFoodStop = dynamic_cast<MessageStop*>(message);
	if (msgFishFoodStop) {
		GameObject* go = msgFishFoodStop->go;
		go->moveSpeed = 0;
	}

	MessageEvolve* msgEvolveFishFood = dynamic_cast<MessageEvolve*>(message);
	if (msgEvolveFishFood) {
		GameObject* go = msgEvolveFishFood->go;
		go->type = GameObject::GO_FISH;
	}

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
					MessageStop msgFishFoodStop = MessageStop(go2);
					go->nearest->Handle(&msgFishFoodStop);
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

void SceneMovement_Week04::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
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

	//On screen text

	std::ostringstream ss;

	ss.precision(3);

	// Exercise Week 4
	// You may wish to add in these codes at the bottom part of SceneMovement_Week04:: Render() to 
	// display the destinations for each GO_FISH in the right panel.


	ss.str("");
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

void SceneMovement_Week04::Exit()
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

// Exercise Week 4
//handle all incoming messages from PostOffice

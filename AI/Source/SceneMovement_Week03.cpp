#include "SceneMovement_Week03.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesFish.h"
#include "StatesShark.h"
#include "StatesBuilding.h"
#include "StatesMechanic.h"
#include "StatesRanged.h"
#include "StatesTank.h"
#include "StatesMortar.h"
#include "StatesScout.h"
#include "GridSettings.h"


#include "StatesAttacker.h"
#include "StatesSupport.h"
#include "SceneData.h"

//0.5882352941 range - circle conversion

SceneMovement_Week03::SceneMovement_Week03()
	: m_turn{}, m_goList{}, m_speed{}, m_gridOffset{}, m_maze{},
	m_start{}, m_end{}, m_myGrid{},
	m_visited{}, m_queue{}, m_previous{},
	m_shortestPath{}, m_mazeKey{}, m_wallLoad{}
{
}

SceneMovement_Week03::~SceneMovement_Week03()
{
}

void SceneMovement_Week03::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	SceneData::GetInstance()->SetObjectCount(0);
	SceneData::GetInstance()->SetFishCount(0);
	SceneData::GetInstance()->SetNumGrid(20);
	SceneData::GetInstance()->SetGridSize(m_worldHeight / SceneData::GetInstance()->GetNumGrid());
	SceneData::GetInstance()->SetGridOffset(SceneData::GetInstance()->GetGridSize() * 0.5f);
	m_gridOffset = m_gridSize / 2;
	m_hourOfTheDay = 0;


	auto* bBase = InitMainBase(GameObject::SIDE_BLUE, Vector3(m_worldWidth - m_gridSize, m_worldHeight / 2, 0.f));
	FillCurrNodes2x2FromWorld(bBase);

	auto* rBase = InitMainBase(GameObject::SIDE_RED, Vector3(0 + m_gridSize, m_worldHeight / 2, 0.f));
	FillCurrNodes2x2FromWorld(rBase);

	float yPull = 5;
	auto* bs1 = InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, (m_worldHeight / 4) + yPull, 0.f));
	FillCurrNodes2x2FromWorld(bs1);

	auto* bs2 = InitSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 4) * 3, (m_worldHeight / 4) * 3 - yPull, 0.f));
	FillCurrNodes2x2FromWorld(bs2);

	auto* rs1 = InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), (m_worldHeight / 4) + yPull, 0.f));
	FillCurrNodes2x2FromWorld(rs1);

	auto* rs2 = InitSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 4), (m_worldHeight / 4) * 3 - yPull, 0.f));
	FillCurrNodes2x2FromWorld(rs2);

	InitMortarSpawner(GameObject::SIDE_BLUE, Vector3((m_worldWidth / 2) + m_gridSize * 3, (m_worldHeight / 2), 0.f));
	InitMortarSpawner(GameObject::SIDE_RED, Vector3((m_worldWidth / 2) - m_gridSize * 3, (m_worldHeight / 2), 0.f));
	//InitGoldenOrb(Vector3(m_worldWidth / 2, m_worldHeight / 2, 0.f));

	gamePlaying = false;

	PostOffice::GetInstance()->Register("Scene", this);


	srand(time(NULL));

	m_start.Set(0, 0);
	m_mazeKey = 2;
	m_wallLoad = 0.3f;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);
	b_grid.resize(m_noGrid * m_noGrid);
	r_grid.resize(m_noGrid * m_noGrid);
	b_visited.resize(m_noGrid * m_noGrid);
	r_visited.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	std::fill(b_grid.begin(), b_grid.end(), Maze::TILE_FOG);
	std::fill(r_grid.begin(), r_grid.end(), Maze::TILE_FOG);
	std::fill(b_visited.begin(), b_visited.end(), false);
	std::fill(r_visited.begin(), r_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;

	Carve2x2Both(0, 9, Maze::TILE_EMPTY);
	Carve2x2Both(4, 5, Maze::TILE_EMPTY);
	Carve2x2Both(4, 13, Maze::TILE_EMPTY);

	Carve2x2Both(18, 9, Maze::TILE_EMPTY);
	Carve2x2Both(14, 5, Maze::TILE_EMPTY);
	Carve2x2Both(14, 13, Maze::TILE_EMPTY);

	//DFS(m_start);
	CarveUntilNoFog();
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
		else if (type == GameObject::GO_GOLDENORB)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateBuildingHealthy("Healthy", go));
			go->sm->AddState(new StateBuildingDeath("Death", go));
		}
		else if (type == GameObject::GO_SPAWNER)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateBuildingHealthy("Healthy", go));
			go->sm->AddState(new StateBuildingDeath("Death", go));
		}
		else if (type == GameObject::GO_SPAWNMORTARAREA)
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
			go->sm->AddState(new StateMechanicHealing("Healing", go));
			go->sm->AddState(new StateMechanicBuilding("Building", go));
			go->sm->AddState(new StateMechanicHurt("Hurt", go));
			go->sm->AddState(new StateMechanicHiding("Hiding", go));
			go->sm->AddState(new StateMechanicGoldenChase("GoldenChase", go));
			go->sm->AddState(new StateMechanicDeath("Death", go));
		}
		else if (type == GameObject::GO_RANGED)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateRangedHealthy("Healthy", go));
			go->sm->AddState(new StateRangedHurt("Hurt", go));
			go->sm->AddState(new StateRangedPanic("Panic", go));
			go->sm->AddState(new StateRangedNearDeath("NearDeath", go));
			go->sm->AddState(new StateRangedDeath("Death", go));
		}	
		else if (type == GameObject::GO_ATTACKER)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateAttackerHealthy("Healthy", go));
			go->sm->AddState(new StateAttackerStayStrong("StayStrong", go));
			go->sm->AddState(new StateAttackerFlee("Flee", go));
			go->sm->AddState(new StateAttackerNearDeath("NearDeath", go));
			go->sm->AddState(new StateAttackerDead("Dead", go));
		}
		else if (type == GameObject::GO_SUPPORT)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None","Healthy", go));
			go->sm->AddState(new StateSupportHealthy("Healthy", go));
			go->sm->AddState(new StateSupportHealing("Healing", go));
			go->sm->AddState(new StateSupportUrgentHealing("UrgentHealing", go));
			go->sm->AddState(new StateSupportHurt("Hurt", go));
			go->sm->AddState(new StateSupportHiding("Hiding", go));
			go->sm->AddState(new StateSupportDeath("Death", go));
		}
		else if (type == GameObject::GO_TANK)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "SoloHealthy", go));
			go->sm->AddState(new StateTankHealthy("Healthy", go));
			go->sm->AddState(new StateTankSoloHealthy("SoloHealthy", go));
			go->sm->AddState(new StateTankSuicide("Suicide", go));
			go->sm->AddState(new StateTankDeath("Death", go));
		}
		else if (type == GameObject::GO_MORTAR)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateMortarHealthy("Healthy", go));
			go->sm->AddState(new StateMortarPanic("Panic", go));
			go->sm->AddState(new StateMortarDeath("Death", go));
		}
		else if (type == GameObject::GO_SCOUT)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateNone("None", "Healthy", go));
			go->sm->AddState(new StateScoutHealthy("Healthy", go));
		}
	}
	return FetchGO(type);
}

GameObject* SceneMovement_Week03::FetchProj(GameObject::GAMEOBJECT_TYPE type)
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
		GameObject* go = new GameObject(type);
		m_projList.push_back(go);
	}
	return FetchProj(type);
}

GameObject* SceneMovement_Week03::InitMainBase(GameObject::SIDE side, Vector3 pos)
{
	GameObject* mainBase = FetchGO(GameObject::GO_MAINBASE);
	mainBase->pos = pos;
	mainBase->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize * 2);
	mainBase->side = side;
	mainBase->target = mainBase->pos;
	mainBase->sm->SetNextState("Healthy");
	mainBase->maxEnergy = 100;
	mainBase->energy = 0;
	mainBase->maxHealth = 900;
	mainBase->health = 900;
	mainBase->moving = false;
	return mainBase;
}

GameObject* SceneMovement_Week03::InitGoldenOrb(Vector3 pos)
{
	GameObject* mainBase = FetchGO(GameObject::GO_GOLDENORB);
	mainBase->pos = pos;
	mainBase->scale = Vector3(m_gridSize * 2, m_gridSize * 2, m_gridSize * 2);
	mainBase->target = mainBase->pos;
	mainBase->sm->SetNextState("Healthy");
	mainBase->maxEnergy = 100;
	mainBase->energy = 0;
	mainBase->maxHealth = 500;
	mainBase->health = 500;
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
	float random = Math::RandFloatMinMax(0.f, 25.f);
	spawner->energy = random;
	spawner->maxEnergy = 100;
	spawner->maxHealth = 700;
	spawner->health = 700;
	spawner->sm->SetNextState("Healthy");
	spawner->woodenLogs = spawner->metalParts = 0;
	spawner->moving = false;
	m_spawners.push_back(spawner);
	return spawner;
}

GameObject* SceneMovement_Week03::InitMortarSpawner(GameObject::SIDE side, Vector3 pos)
{
	GameObject* spawner = FetchGO(GameObject::GO_SPAWNMORTARAREA);
	spawner->pos = pos;
	spawner->scale = Vector3(0,0,0);
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
	int gx, gy;

	while (true)
	{
		// pick a grid offset between -3 and +3
		gx = Math::RandIntMinMax(-2, 2);
		gy = Math::RandIntMinMax(-2, 2);

		// exclude the spawner 2×2 area: gx in [-1, 0], gy in [-1, 0]
		bool insideSpawner =
			(gx >= -1 && gx <= 0 &&
				gy >= -1 && gy <= 0);

		if (!insideSpawner)
			break;
	}

	// convert grid offsets → world position centered on that grid cell
	float x = center.x + gx * m_gridSize + m_gridSize * 0.5f;
	float y = center.y + gy * m_gridSize + m_gridSize * 0.5f;

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
		unit = FetchGO(GameObject::GO_ATTACKER);
		unit->type = GameObject::GO_ATTACKER;
		unit->viewRange = 2;
		unit->atkRange = 1;
		unit->useMoves = 3;
	}
	else if ((useRandom && random < AttackerRate + RangedRate && random > AttackerRate) || type == GameObject::GO_RANGED)
	{
		unit = FetchGO(GameObject::GO_RANGED);
		unit->type = GameObject::GO_RANGED;
		unit->viewRange = 4;
		unit->atkRange = 3;
	}
	else if ((useRandom && random < AttackerRate + RangedRate + SupportRate && random > AttackerRate + RangedRate) || type == GameObject::GO_SUPPORT)
	{
		unit = FetchGO(GameObject::GO_SUPPORT);
		unit->type = GameObject::GO_SUPPORT;
		unit->viewRange = 3;
		unit->atkRange = 2;
	}
	else if ((useRandom && random < 100 && random > AttackerRate + RangedRate + SupportRate) || type == GameObject::GO_MECHANIC)
	{
		unit = FetchGO(GameObject::GO_MECHANIC);
		unit->type = GameObject::GO_MECHANIC;
		unit->viewRange = 2;
		unit->atkRange = 1;
	}
	else if ((type == GameObject::GO_SCOUT))
	{
		unit = FetchGO(GameObject::GO_SCOUT);
		unit->type = GameObject::GO_SCOUT;
		unit->viewRange = 2;
		unit->atkRange = 1;
	}

	// i need to set a blank state first, in order to access the Enter() of the first actual state
	unit->sm->SetNextState("None");
	unit->maxHealth = 300;
	unit->health = 300;
	unit->maxEnergy = 10;
	unit->energy = 0;

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

	unit->sm->SetNextState("None");
	unit->maxHealth = 400;
	unit->health = 400;
	unit->maxEnergy = 10;
	unit->energy = 0;

	unit->scale = Vector3(m_gridSize, m_gridSize, m_gridSize);
	unit->side = side;
	unit->pos = pos;
	unit->target = unit->pos;
	unit->moveSpeed = 5.f;
	unit->moving = true;
	return unit;
}

int SceneMovement_Week03::GetHealPriority(GameObject* target)
{
	if (!target || !target->active)
		return -999;

	// --- TOP PRIORITY (near death) ---
	if (target->sm->GetCurrentState() == "NearDeath")
		return 1000;


	if (target->type == GameObject::GO_RANGED && target->sm->GetCurrentState() == "Panic")
		return 300;

	// --- High priority cases ---
	if (target->type == GameObject::GO_ATTACKER)
	{
		std::string s = target->sm->GetCurrentState();
		if (s == "Flee") return 600;
		if (s == "StayStrong") return 500;
		return 400; // healthy attacker but still important
	}

	// Mechanic → helps spawners (important)
	if (target->type == GameObject::GO_MECHANIC)
		return 350;

	// Ranged → squishy DPS
	if (target->type == GameObject::GO_RANGED)
		return 300;

	// Tanks → usually lower need
	if (target->type == GameObject::GO_TANK)
		return 200;

	// Other allies
	return 100;
}

void SceneMovement_Week03::SpawnGrid3x3(GameObject::SIDE side,
	const Vector3& centerPos,
	GameObject::GAMEOBJECT_TYPE type)
{
	float half = m_gridSize * 0.5f;

	// Offsets for a 3×3 block: -1, 0, +1
	for (int gx = -1; gx <= 1; gx++)
	{
		for (int gy = -1; gy <= 1; gy++)
		{
			float x = centerPos.x + gx * m_gridSize + half;
			float y = centerPos.y + gy * m_gridSize + half;

			Vector3 spawnPos(x, y, centerPos.z);

			GameObject* unit = SpawnUnit(side, spawnPos, type);

			// optional: activate, initialize, or give orders
			// unit->moving = true;
		}
	}
}

Vector3 SceneMovement_Week03::SpawnMortarArea(GameObject::SIDE side)
{
	Vector3 center;
	for(std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
		if (go->type == GameObject::GO_SPAWNMORTARAREA && go->side == side)
		{
			center = go->pos;
		}
	}
	float halfWidthTiles = 0.75;
	float halfHeightTiles = 1.25;
	// pick random tile offset within [-halfWidthTiles, +halfWidthTiles]
	int gx = Math::RandIntMinMax(-halfWidthTiles, halfWidthTiles);
	int gy = Math::RandIntMinMax(-halfHeightTiles, halfHeightTiles);

	// convert tile offset → world space
	float x = center.x + gx * m_gridSize + m_gridSize * 0.5f;
	float y = center.y + gy * m_gridSize + m_gridSize * 0.5f;

	return Vector3(x, y, center.z);
}

SceneMovement_Week03::ArmyStats SceneMovement_Week03::ComputeArmyStats(GameObject::SIDE mySide)
{
	ArmyStats stats;
	
	MessageHowManyUnit msg = MessageHowManyUnit(GameObject::GO_ATTACKER, mySide);
	stats.countPerType[(int)GameObject::GO_ATTACKER] = HandleCount(&msg);
	msg = MessageHowManyUnit(GameObject::GO_RANGED, mySide);
	stats.countPerType[(int)GameObject::GO_RANGED] = HandleCount(&msg);
	msg = MessageHowManyUnit(GameObject::GO_SUPPORT, mySide);
	stats.countPerType[(int)GameObject::GO_SUPPORT] = HandleCount(&msg);
	msg = MessageHowManyUnit(GameObject::GO_MECHANIC, mySide);
	stats.countPerType[(int)GameObject::GO_MECHANIC] = HandleCount(&msg);
	msg = MessageHowManyUnit(GameObject::GO_TANK, mySide);
	stats.countPerType[(int)GameObject::GO_TANK] = HandleCount(&msg);
	msg = MessageHowManyUnit(GameObject::GO_MORTAR, mySide);
	stats.countPerType[(int)GameObject::GO_MORTAR] = HandleCount(&msg);

	for (int i = 0; i < (int)GameObject::GO_TOTAL; ++i)
	{
		stats.total += stats.countPerType[i];
	}

	stats.side = mySide;

	return stats;
}

int SceneMovement_Week03::MechanicNeedGet(GameObject* spawner)
{
	ArmyStats stats = ComputeArmyStats(spawner->side);

	int total = stats.total; //if 0, make it atleast 1
	if (total <= 0)
		total = 1;

	// all the current ratios
	float rAttacker = stats.countPerType[(int)GameObject::GO_ATTACKER] / (float)total;
	float rRanged = stats.countPerType[(int)GameObject::GO_RANGED] / (float)total;
	float rSupport = stats.countPerType[(int)GameObject::GO_SUPPORT] / (float)total;
	float rMech = stats.countPerType[(int)GameObject::GO_MECHANIC] / (float)total;
	float rTank = stats.countPerType[(int)GameObject::GO_TANK] / (float)total;
	float rMortar = stats.countPerType[(int)GameObject::GO_MORTAR] / (float)total;

	// how much more i need, 0 = enough
	//just clamping to 0
	float needAttacker = (((0.0f) > (attackerRatio - rAttacker)) ? (0.0f) : (attackerRatio - rAttacker));
	float needRanged = (((0.0f) > (rangedRatio - rRanged)) ? (0.0f) : (rangedRatio - rRanged));
	float needSupport = (((0.0f) > (supportRatio - rSupport)) ? (0.0f) : (supportRatio - rSupport));
	float needMech = (((0.0f) > (mechanicRatio - rMech)) ? (0.0f) : (mechanicRatio - rMech));
	float needTank = (((0.0f) > (tankRatio - rTank)) ? (0.0f) : (tankRatio - rTank));
	float needMortar = (((0.0f) > (mortarRatio - rMortar)) ? (0.0f) : (mortarRatio - rMortar));

	//find avg need
	float woodNeed = (needAttacker + needRanged + needSupport + needMech) / 4.0f;
	float metalNeed = ((needTank + needMortar) / 2.0f) * 0.5f;

	// ---- Also look at current stock, so we don't overcap one resource ----
	int woodStock = spawner->woodenLogs;
	int metalStock = spawner->metalParts;

	if(metalStock > highMetalThreshold && woodStock < highWoodThreshold)
		return 1;
	if (woodStock > highWoodThreshold && metalStock < highMetalThreshold)
		return 2;

	// If one resource is low while the other is okay, force that
	if (woodStock < lowWoodThreshold && metalStock > lowMetalThreshold)
		return 1;

	if (metalStock < lowMetalThreshold && woodStock > lowWoodThreshold)
		return 2;

	//if both needs are 0, then balance
	if (woodNeed <= 0.001f && metalNeed <= 0.001f)
	{
		if (woodStock < metalStock)
			return 1;
		else
			return 2;
	}
	// Otherwise, pick the resource with higher "need"
	if (woodNeed >= metalNeed)
		return 1;
	else
		return 2;

	return 0;
}

bool SceneMovement_Week03::DecideSpawn(GameObject* spawner)
{
	ArmyStats stats = ComputeArmyStats(spawner->side);

	int total = stats.total; //if 0, make it atleast 1
	if (total <= 0)
		total = 1;

	// all the current ratios
	float rAttacker = stats.countPerType[(int)GameObject::GO_ATTACKER] / (float)total;
	float rRanged = stats.countPerType[(int)GameObject::GO_RANGED] / (float)total;
	float rSupport = stats.countPerType[(int)GameObject::GO_SUPPORT] / (float)total;
	float rMech = stats.countPerType[(int)GameObject::GO_MECHANIC] / (float)total;
	float rTank = stats.countPerType[(int)GameObject::GO_TANK] / (float)total;
	float rMortar = stats.countPerType[(int)GameObject::GO_MORTAR] / (float)total;

	// how much more i need, 0 = enough
	//just clamping to 0
	float needAttacker = (((0.0f) > (attackerRatio - rAttacker)) ? (0.0f) : (attackerRatio - rAttacker));
	float needRanged = (((0.0f) > (rangedRatio - rRanged)) ? (0.0f) : (rangedRatio - rRanged));
	float needSupport = (((0.0f) > (supportRatio - rSupport)) ? (0.0f) : (supportRatio - rSupport));
	float needMech = (((0.0f) > (mechanicRatio - rMech)) ? (0.0f) : (mechanicRatio - rMech));
	float needTank = (((0.0f) > (tankRatio - rTank)) ? (0.0f) : (tankRatio - rTank));
	float needMortar = (((0.0f) > (mortarRatio - rMortar)) ? (0.0f) : (mortarRatio - rMortar));

	//special cases
	if (spawner->woodenLogs < lowWoodThreshold || spawner->metalParts < lowMetalThreshold)
	{
		needMech += 0.2f;
	}

	// Helper 
	auto TypeName = [](GameObject::GAMEOBJECT_TYPE t) -> const char*
		{
			switch (t)
			{
			case GameObject::GO_ATTACKER: return "ATTACKER";
			case GameObject::GO_RANGED:   return "RANGED";
			case GameObject::GO_SUPPORT:  return "SUPPORT";
			case GameObject::GO_MECHANIC: return "MECHANIC";
			case GameObject::GO_TANK:     return "TANK";
			case GameObject::GO_MORTAR:   return "MORTAR";
			default:                      return "UNKNOWN";
			}
		};

	// scores for debug; start with "invalid" value
	float attackerScore = -99999.0f;
	float rangedScore = -99999.0f;
	float supportScore = -99999.0f;
	float mechanicScore = -99999.0f;
	float tankScore = -99999.0f;
	float mortarScore = -99999.0f;

	GameObject::GAMEOBJECT_TYPE bestType = GameObject::GO_ATTACKER;
	float bestScore = -99999.0f;

	// ATTACKER (wood)
	if (spawner->woodenLogs >= costAttacker)
	{
		float costNorm = costAttacker / maxCost;
		float score = needAttacker * 10.0f - costWeight * costNorm;
		attackerScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_ATTACKER;
		}
	}

	// RANGED (wood)
	if (spawner->woodenLogs >= costRanged)
	{
		float costNorm = costRanged / maxCost;
		float score = needRanged * 10.0f - costWeight * costNorm;
		rangedScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_RANGED;
		}
	}

	// SUPPORT (wood)
	if (spawner->woodenLogs >= costSupport)
	{
		float costNorm = costSupport / maxCost;
		float score = needSupport * 10.0f - costWeight * costNorm;
		supportScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_SUPPORT;
		}
	}

	// MECHANIC (wood)
	if (spawner->woodenLogs >= costMech)
	{
		float costNorm = costMech / maxCost;
		float score = needMech * 10.0f - costWeight * costNorm;
		mechanicScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_MECHANIC;
		}
	}

	// TANK (metal)
	if (spawner->metalParts >= costTank)
	{
		float costNorm = costTank / maxCost;
		float score = needTank * 10.0f - costWeight * costNorm;
		tankScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_TANK;
		}
	}

	// MORTAR (metal)
	if (spawner->metalParts >= costMortar)
	{
		float costNorm = costMortar / maxCost;
		float score = needMortar * 10.0f - costWeight * costNorm;
		mortarScore = score;

		if (score > bestScore)
		{
			bestScore = score;
			bestType = GameObject::GO_MORTAR;
		}
	}

	// If nothing was affordable
	if (bestScore <= -99998.0f)
	{
		return false;
	}

	// Debug: print all scores + chosen type
	//std::cout << "Scores: "
	//	<< "A:" << attackerScore
	//	<< " R:" << rangedScore
	//	<< " S:" << supportScore
	//	<< " Me:" << mechanicScore
	//	<< " T:" << tankScore
	//	<< " Mo:" << mortarScore
	//	<< " | Wood:" << spawner->woodenLogs
	//	<< " Metal:" << spawner->metalParts
	//	<< " | Chosen: " << TypeName(bestType)
	//	<< " (bestScore=" << bestScore << ")"
	//	<< std::endl << std::endl;

	Vector3 randomPos = RandomPointInRing(spawner->pos, 3.75, 10);
	switch (bestType)
	{
	case GameObject::GO_ATTACKER: spawner->woodenLogs -= costAttacker; break;
	case GameObject::GO_RANGED:   spawner->woodenLogs -= costRanged;   break;
	case GameObject::GO_SUPPORT:  spawner->woodenLogs -= costSupport;  break;
	case GameObject::GO_MECHANIC: spawner->woodenLogs -= costMech;     break;
	case GameObject::GO_TANK:     spawner->metalParts -= costTank;     break;
	case GameObject::GO_MORTAR:   spawner->metalParts -= costMortar;   break;
	default: break;
	}

	if (bestType == GameObject::GO_ATTACKER ||
		bestType == GameObject::GO_RANGED ||
		bestType == GameObject::GO_SUPPORT ||
		bestType == GameObject::GO_MECHANIC)
	{
		SpawnUnit(spawner->side, randomPos, bestType);
	}
	// Metal-based units → SpawnMetalUnit
	else if (bestType == GameObject::GO_TANK)
	{
		SpawnMetalUnit(spawner->side, randomPos, bestType);
	}	
	else if (bestType == GameObject::GO_MORTAR)
	{
		MessageMechanicBuild msg = MessageMechanicBuild(spawner);
		Handle(&msg);
	}
	return true;
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
	//m_worldHeight = 100.f;
	//m_worldWidth = m_worldHeight;

	//m_gridSize = SceneData::GetInstance()->GetGridSize();
	//m_gridOffset = SceneData::GetInstance()->GetGridOffset();
	//m_noGrid = SceneData::GetInstance()->GetNumGrid();

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
		//std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize) //ensure we're clicking within the board
		{
			m_end.Set(static_cast<int>(posX / m_gridSize), static_cast<int>(posY / m_gridSize));
			std::cout << static_cast<int>(posX / m_gridSize) << " , " << static_cast<int>(posY / m_gridSize) << std::endl;
			for (GameObject* go : m_goList)
				if (go->active && go->type == GameObject::GO_NPC)
					PathFind(go, m_end, 1,0);
		}
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		//std::cout << "LBUTTON UP" << std::endl;
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
		if (!gamePlaying) {
			//Vector3 randomPos = RandomPointInRing(m_spawners[0]->pos, 3.75, 10);
			GameObject* uni1 = SpawnUnit(GameObject::SIDE_BLUE, Vector3(0,0,0), GameObject::GO_ATTACKER);
			uni1->curr.Set(15, 5);
			RevealAround(uni1,uni1->viewRange);
			uni1->stack.push_back(uni1->curr); //triggers dfs
			b_visited[Get1DIndex(15, 5)] = true;

			uni1 = SpawnUnit(GameObject::SIDE_RED, Vector3(0, 0, 0), GameObject::GO_ATTACKER);
			uni1->curr.Set(4, 5);
			RevealAround(uni1, uni1->viewRange);
			uni1->stack.push_back(uni1->curr); //triggers dfs
			b_visited[Get1DIndex(4, 5)] = true;
			//randomPos = RandomPointInRing(m_spawners[1]->pos, 3.75, 10);
			//uni1 = SpawnUnit(GameObject::SIDE_BLUE, randomPos, GameObject::GO_MECHANIC);
			//randomPos = RandomPointInRing(m_spawners[2]->pos, 3.75, 10);
			//uni1 = SpawnUnit(GameObject::SIDE_RED, randomPos, GameObject::GO_MECHANIC);
			//randomPos = RandomPointInRing(m_spawners[3]->pos, 3.75, 10);
			//uni1 = SpawnUnit(GameObject::SIDE_RED, randomPos, GameObject::GO_MECHANIC);
			//gamePlaying = true;
		}

	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
	static bool bVState = false;
	if (!bVState && Application::IsKeyPressed('V'))
	{
		bVState = true;

		//Vector3 randomPos = RandomPointInRing(m_spawners[2]->pos, 3.75, 10);
		//SpawnGrid3x3(GameObject::SIDE_RED, randomPos, GameObject::GO_ATTACKER);

	}
	else if (bVState && !Application::IsKeyPressed('V'))
	{
		bVState = false;
	}
	//static bool bRState = false;
	//if (!bRState && Application::IsKeyPressed('R'))
	//{
	//	bRState = true;
	//}
	//else if (bRState && !Application::IsKeyPressed('R'))
	//{
	//	bRState = false;
	//	Exit();
	//	Init();
	//	return;
	//}
	//static bool bFState = false;
	//if (!bFState && Application::IsKeyPressed('F'))
	//{
	//	bFState = true;

	//	bool found = false;
	//	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	//	{
	//		GameObject* go = (GameObject*)*it;
	//		if (!go->active)
	//			continue;
	//		if (go->type == GameObject::GO_MECHANIC)
	//		{
	//			if (go->specID == 2) {
	//				found = true;
	//				go->health -= 10;
	//			}
	//		}
	//	}

	//	if (!found)
	//	{
	//		Vector3 randomPos = RandomPointInRing(m_spawners[2]->pos, 3.75, 10);
	//		GameObject* uni1 = SpawnUnit(GameObject::SIDE_RED, randomPos, GameObject::GO_MECHANIC);
	//		uni1->specID = 2;
	//	}
	//}
	//else if (bFState && !Application::IsKeyPressed('F'))
	//{
	//	bFState = false;
	//}
	static bool bGState = false;
	if (!bGState && Application::IsKeyPressed('G'))
	{
		bGState = true;

		//Vector3 randomPos = RandomPointInRing(m_spawners[2]->pos, 3.75, 10);
		//GameObject* uni3 = SpawnUnit(GameObject::SIDE_RED, randomPos, GameObject::GO_ATTACKER);
		//ref = uni3;
	}
	else if (bGState && !Application::IsKeyPressed('G'))
	{
		bGState = false;
	}

	//if (timeCounter >= 300) { gamePlaying = false; }
	//if (!gamePlaying) return;

	static float checkTimer = 0.0f;

	//----------GoldenOrb Event Spawn--------------
	
	//if (timeCounter >= 30 && !goldenEvent)
	//{
	//	checkTimer += dt;                 // dt = seconds per frame
	//	if (checkTimer >= 1.0f)           // check once per second
	//	{
	//		checkTimer -= 1.0f;
	//		float r = Math::RandFloatMinMax(0.f, 1.f);
	//		if (r < 0.05f)
	//		{
	//			InitGoldenOrb(Vector3(m_worldWidth * 0.5f, m_worldHeight * 0.5f, 0.f));
	//			goldenEvent = true;
	//		}
	//	}
	//	if (timeCounter >= 80 && !goldenEvent) //force spawn if too long
	//	{
	//		InitGoldenOrb(Vector3(m_worldWidth * 0.5f, m_worldHeight * 0.5f, 0.f));
	//		goldenEvent = true;
	//	}
	//}

	//StateMachine
	for (int i = 0; i < m_goList.size(); i++) {
		GameObject* go = m_goList[i];
		if (!go->active)
			continue;
		if (go->sm)
			go->sm->Update(dt);
	}
	//if (ref != NULL && ref->nearest != NULL)
		//std::cout << ref->nearest << std::endl;
		

	for (int i = 0; i < m_goList.size();i++) {
		if (!m_goList[i]->active) continue;
		if (m_goList[i]->type == GameObject::GO_ATTACKER)
			std::cout << "1 " << m_goList[i]->health << std::endl;
	}

	//External triggers
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;
 		
		if (go->type == GameObject::GO_SPAWNER)
		{
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go->sm->GetCurrentState() == "Death")
				{
					MessageWRU msgAreaDamage = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 1.5f);
					Handle(&msgAreaDamage);
					for (auto& target : go->hits)
					{
						target->health -= 20;
						target->lastAttacker = go;
					}
				}
			}
		}
		else if (go->type == GameObject::GO_MECHANIC)
		{
			//std::cout << "MECHANIC STATE: " << go->sm->GetCurrentState() << std::endl;
			if (go->sm->GetCurrentState() == "Healthy")
			{
				MessageWRU msgCheckSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200.0f);
				Handle(&msgCheckSpawner);
				if (go->nearest != nullptr) {
					if (go->nearest->active)
					{
						if (go->nearest->type == GameObject::GO_SPAWNER) {
							go->choice = MechanicNeedGet(go->nearest);
						}
						else { go->choice = 0; }
					}
				}
			}

			if(go->sm->GetCurrentState() == "Hurt")
			{
				MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
				Handle(&nearestSpawner);
				MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FREE_SUP, 200.0f);
				Handle(&msgNeedSup);
				if (go->external != NULL) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}
			}
			go->steps = 0;
			GameObject* temp = go->nearest;
			MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
			Handle(&nearestSpawner);
			MessageWRU amtOfAtk = MessageWRU(go, MessageWRU::SEARCH_TYPE::ATTACKERCOUNT, 200);
			Handle(&amtOfAtk);
			go->nearest = temp;
			if (go->sm->GetCurrentState() == "Hiding")
			{
				MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
				Handle(&nearestSpawner);
			}

			if (go->sm->GetCurrentState() == "Building")
			{
				if (go->normalTarget == Vector3(0, 0, 0))
					go->normalTarget = SpawnMortarArea(go->side);
			}

			MessageWRU goldenOrb = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_GOLDENORB, 500.0f);
			Handle(&goldenOrb);
			if (go->sm->GetCurrentState() == "GoldenChase")
			{
				GameObject* temp = go->nearest;
				MessageWRU msgCheckSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 50.0f);
				Handle(&msgCheckSpawner);

				if (go->nearest != nullptr) {
					if (go->nearest->active) {
						if (go->nearest->type == GameObject::GO_SPAWNER) {
							go->choice = MechanicNeedGet(go->nearest);
						}
						else { go->choice = 0; }
					}
				}
				else { go->choice = 0; }
				go->external2 = go->nearest;
				go->nearest = temp;
			}
		}
		else if (go->type == GameObject::GO_RANGED)
		{
			//std::cout << "RANGED STATE: " << go->sm->GetCurrentState() << std::endl;
			if (go->sm->GetCurrentState() == "Healthy")
			{
				//if no target, or if current target died
				if (go->nearest == NULL || go->nearest->active == false || go->nearest->hiding) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 500.0f);
					Handle(&msgCheckEnemy);
				}
				else //use for radius to check whether enemy is too close
				{
					GameObject* temp = go->nearest;
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
					go->countDown = (go->nearest->pos - go->pos).Length();
					go->nearest = temp;
					if (go->countDown < m_gridSize * 2) //if target too close
					{
						continue;
					}
					else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
					{
						continue;
					}
					else
					{
						go->moving = true;
						MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
						Handle(&msgCheckEnemy);
						go->normalTarget = go->nearest->pos;
					}
				}
			}
			else if (go->sm->GetCurrentState() == "Hurt" || go->sm->GetCurrentState() == "Panic" || go->sm->GetCurrentState() == "NearDeath") {
				if(go->sm->GetCurrentState() != "NearDeath")
				{
					MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FREE_SUP, 200.0f);
					Handle(&msgNeedSup);
					if (go->external != NULL) {
						if (go->external->active) {
							MessageAskHelp msgAskHelp = MessageAskHelp(go);
							go->external->Handle(&msgAskHelp);
						}
					}
				}
				else
				{
					//find nearest free support
					MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_URG_SUP, 200.0f);
					Handle(&msgNeedSup);
					if (go->external != NULL) {
						if (go->external->active) {
							MessageAskHelp msgAskHelp = MessageAskHelp(go);
							go->external->Handle(&msgAskHelp);
						}
					}
				}
				if(go->sm->GetCurrentState() != "Hurt")//give ranged a last attacker for testing
				{
					//go->lastAttacker = ref;
					//find nearest spawner
					MessageWRU msgNeedAtk = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ALLY_ATTACKER, 200.0f);
					Handle(&msgNeedAtk);
					if (go->external2 != NULL) {
						if (go->external2->active) {
							MessageAskForAtk msgAskHelp = MessageAskForAtk(go);
							go->external2->Handle(&msgAskHelp);
						}
					}
				}

				//if no target, or if current target died
				if (go->nearest == NULL || go->nearest->active == false || go->nearest->hiding) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
				else //use for radius to check whether enemy is too close
				{
					GameObject* temp = go->nearest;
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
					go->countDown = (go->nearest->pos - go->pos).Length();
					go->nearest = temp;

					int distanceaway;
					if (go->sm->GetCurrentState() == "Panic") { distanceaway = 4; }
					else if (go->sm->GetCurrentState() == "Hurt") { distanceaway = 3; }	
					else { distanceaway = 4; }
					if (go->countDown < m_gridSize * distanceaway) //if target too close
					{
						continue;
					}
					else if ((go->nearest->pos - go->pos).Length() < m_gridSize * 6)
					{
						continue;
					}
					else
					{
						//std::cout << "moving closer!" << std::endl;
						go->moving = true;
						MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
						Handle(&msgCheckEnemy);
						go->normalTarget = go->nearest->pos;
					}
				}
			}
		}
		else if (go->type == GameObject::GO_ATTACKER) //REMEMBERRRRRRRR TELL SUPPORTER TO GO AWAY ONCE HEALED //also a chance, unit back off to spawner but still getting hit
		{
			if (go->side == GameObject::SIDE_RED) {
				//std::cout << "Attacker State: " << go->sm->GetCurrentState() << std::endl;
			}
			if (go->sm->GetCurrentState() == "Healthy") {
				//if no target, or if current target died
				if (go->nearest == NULL || go->nearest->active == false || go->nearest->hiding) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
				else
				{
					if ((go->nearest->pos - go->pos).Length() < m_gridSize)
					{
						continue;
					}
					else if (go->urgent) { //if im not close enough, continue checking
						continue;
					}
					else {
						go->moving = true;
						MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
						Handle(&msgCheckEnemy);
					}
				}
			}
			else if (go->sm->GetCurrentState() == "Flee") {

				int step = go->steps;
				go->steps = 0;
				GameObject* temp = go->nearest;
				MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
				Handle(&nearestSpawner);
				MessageWRU amtOfAtk = MessageWRU(go, MessageWRU::SEARCH_TYPE::ATTACKERCOUNT, 200);
				Handle(&amtOfAtk);
				if (go->steps > 1) //if atleast 2 attacking my base while im behind
				{
					go->sm->SetNextState("StayStrong");
				}
				go->steps = step;
				go->nearest = temp;

				//find nearest free support
				MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FREE_SUP, 200.0f);
				Handle(&msgNeedSup);
				if (go->external != NULL ) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}

				if (go->nearest == NULL || go->nearest->active == false) { //find nearest spawner
					MessageWRU msgCheckSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200.0f);
					Handle(&msgCheckSpawner);
					go->steps = Math::RandIntMinMax(-1,1);
					if (go->steps == 0) { go->steps = 1; } go->steps = 1;
				}
			}
			else if (go->sm->GetCurrentState() == "StayStrong") {

				//find nearest free support
				MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_FREE_SUP, 200.0f);
				Handle(&msgNeedSup);
				if (go->external != NULL) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}

				//NORMAL ATTACKING BEHAVIOUR
				//if no target, or if current target died
				if (go->nearest == NULL || go->nearest->active == false || go->nearest->hiding) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
				else
				{
					if ((go->nearest->pos - go->pos).Length() < m_gridSize)
					{
						continue;
					}
					else { //if im not close enough, continue checking
						go->moving = true;
						MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
						Handle(&msgCheckEnemy);
					}
				}
			}
			else if (go->sm->GetCurrentState() == "NearDeath") {

				int step = go->steps;
				go->steps = 0;
				GameObject* temp = go->nearest;
				MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
				Handle(&nearestSpawner);
				MessageWRU amtOfAtk = MessageWRU(go, MessageWRU::SEARCH_TYPE::ATTACKERCOUNT, 200);
				Handle(&amtOfAtk);
				if (go->steps > 1) //if atleast 2 attacking my base while im behind
				{
					go->sm->SetNextState("StayStrong");
				}
				go->steps = step;
				go->nearest = temp;

				//find nearest free support
				MessageWRU msgNeedSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_URG_SUP, 200.0f);
				Handle(&msgNeedSup);
				if (go->external != NULL) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}

				if (go->nearest == NULL || go->nearest->active == false) { //find nearest spawner
					MessageWRU msgCheckSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200.0f);
					Handle(&msgCheckSpawner);
					go->steps = Math::RandIntMinMax(-1, 1);
					if (go->steps == 0) { go->steps = 1; } go->steps = 1;
				}
			}
		}
		else if (go->type == GameObject::GO_SUPPORT) 
		{
			//std::cout << "Supporter State: " << go->sm->GetCurrentState() << std::endl;

			go->alliesActiveCount = 0;
			MessageWRU allyCount = MessageWRU(go, MessageWRU::SEARCH_TYPE::ALLYACTIVECOUNT, 500);
			Handle(&allyCount);

			go->external = go->nearest; // external is to store my past frame target for support
			if (go->sm->GetCurrentState() == "Healthy") {
				if (go->nearest == NULL || go->nearest->active == false) {
					MessageWRU nearestAllyNoSup = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ALLY_NOSUP, 200);
					Handle(&nearestAllyNoSup);
				}
			}
			if (go->external != NULL) {
				if (go->external != go->nearest)
				{
					go->external->supportActionSpeed = 0;
					go->external->supportSpeed = 0;
				}
			}
			if (go->sm->GetCurrentState() == "Hurt" || go->sm->GetCurrentState() == "Hiding") {
				MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
				Handle(&nearestSpawner);
			}
		}
		else if (go->type == GameObject::GO_TANK)
		{
			//std::cout << "TANK STATE: " << go->nearest << std::endl;
			if (go->sm->GetCurrentState() == "Healthy") {
				if (go->external2 == NULL || go->external2->active == false) {
					GameObject* temp = go->nearest;
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
					go->external2 = go->nearest;
					go->nearest = temp;
				}
				//tank uses target more instead of nearest. because to prevent tanks stacking on top of each other at the front
				if (go->nearest == NULL || go->nearest->active == false) {
					MessageWRU msgCheckFrontline = MessageWRU(go, MessageWRU::SEARCH_TYPE::FURTHEST_FRONTLINE, 200);
					Handle(&msgCheckFrontline);
					go->steps = Math::RandIntMinMax(-2, 2);
					if(go->nearest == NULL)
					{
						go->sm->SetNextState("SoloHealthy");
						return; // or handle “no frontline found” however you want
					}
				}
				else
				{
					MessageWRU msgCheckFrontline = MessageWRU(go, MessageWRU::SEARCH_TYPE::FURTHEST_FRONTLINE, 200);
					Handle(&msgCheckFrontline);
					int redOrBlue = (go->side == GameObject::SIDE_BLUE) ? -1 : 1;
					go->normalTarget = go->nearest->pos + Vector3(redOrBlue * m_gridSize * 1.85f, (float)go->steps * m_gridSize, 0);
					if ((go->normalTarget - go->pos).Length() < m_gridSize)
					{
						go->moving = false;
					}
					else { //if im not there yet, continue checking
						go->moving = true;
					}
				}
			}
			else if (go->sm->GetCurrentState() == "SoloHealthy") {
				if (go->nearest == NULL || go->nearest->active == false) {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
				GameObject* temp = go->nearest;
				go->nearest = NULL;
				MessageWRU msgCheckFrontline = MessageWRU(go, MessageWRU::SEARCH_TYPE::FURTHEST_FRONTLINE, 200);
				Handle(&msgCheckFrontline);
				if (go->nearest != NULL)
				{
					go->sm->SetNextState("Healthy");
					return;
				}
				go->nearest = temp;
				if ((go->nearest->pos - go->pos).Length() < m_gridSize * 5)
				{
					continue;
				}
				else {
					MessageWRU msgCheckEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 200.0f);
					Handle(&msgCheckEnemy);
				}
			}
			else if (go->sm->GetCurrentState() == "Suicide")
			{
				MessageWRU msgCheckObj = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_OBJ, 200.0f);
				Handle(&msgCheckObj);
				if (go->countDown <= 0)
				{
					MessageWRU msgAreaDamage = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 1.5f);
					Handle(&msgAreaDamage);
					for (auto& target : go->hits)
					{
						target->health -= 20;
						target->lastAttacker = go;
					}
				}
			}

			if (go->panicking)
			{
				MessageWRU msgCheckMech = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_MECHANIC_HEAL, 200.0f);
				Handle(&msgCheckMech);
				if (go->external != NULL) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}
			}
		}
		else if (go->type == GameObject::GO_MORTAR)
		{
			MessageWRU nearestMortarEnemy = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_MORTAR_ENEMY, 35,10);
			Handle(&nearestMortarEnemy);
			if(go->nearest != NULL)
			{
				float finalActionSpeed = (go->actionSpeed * 1) + go->supportActionSpeed;
				if (go->EnergyReduce(finalActionSpeed))
				{
					PostOffice::GetInstance()->Send("Scene", new MessageSpawnMorBomb(go));
				}
			}
			if (go->sm->GetCurrentState() == "Panic")
			{
				MessageWRU msgCheckMech = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_MECHANIC_HEAL, 200.0f);
				Handle(&msgCheckMech);
				if (go->external != NULL) {
					if (go->external->active) {
						MessageAskHelp msgAskHelp = MessageAskHelp(go);
						go->external->Handle(&msgAskHelp);
					}
				}
			}
		}
	}


	//loop through all spawners
	for (std::vector<GameObject*>::iterator it = m_spawners.begin(); it != m_spawners.end(); ++it)
	{
		GameObject* spawner = *it;
		if (spawner->active == false) { continue; }
		spawner->energy += 0.13; // dont wanna use EnergyReduce function, i want energy to stop at max

		if (spawner->energy >= spawner->maxEnergy)
		{
			if (DecideSpawn(spawner))
			{
				spawner->energy = 0;
			}
			else
			{
				spawner->energy = spawner->maxEnergy;
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
		if (proj->type == GameObject::GO_PROJECTILE) {

			MessageWRU msgCheckEnemy = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 50.0f);
			Handle(&msgCheckEnemy);
			if (proj->nearest != NULL) {
				float distance = (proj->target - proj->pos).Length();
				float closestDistance = (proj->nearest->pos - proj->pos).Length();

				if (closestDistance < proj->nearest->scale.x) { //if it hits something while flying
					proj->health = 0;
					proj->type = GameObject::GO_NONE;
					proj->active = false;
					proj->nearest->health -= 5;
					proj->nearest->lastAttacker = proj->owner;
				}
				if (distance <= 0.001)
				{
					proj->health = 0;
					proj->type = GameObject::GO_NONE;
					proj->active = false;
				}
			}
		}
		else if (proj->type == GameObject::GO_ROCKETS) {

			MessageWRU msgCheckEnemy = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY, 50.0f);
			Handle(&msgCheckEnemy);
			if (proj->nearest != NULL) {
				float distance = (proj->target - proj->pos).Length();
				float closestDistance = (proj->nearest->pos - proj->pos).Length();

				if (closestDistance < proj->nearest->scale.x) { //if it hits something while flying
					proj->health = 0;
					proj->type = GameObject::GO_NONE;
					proj->active = false;
					proj->hits.clear();
					proj->pos = proj->nearest->pos; //set pos to target pos for area damage search
					MessageWRU msgAreaDamage = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 1.5f);
					Handle(&msgAreaDamage);
					for(auto& target : proj->hits)
					{
						target->health -= 10;
						target->lastAttacker = proj->owner;
					}
				}
				if (distance <= 0.001)
				{
					proj->health = 0;
					proj->type = GameObject::GO_NONE;
					proj->active = false;
				}
			}
		}
		else if (proj->type == GameObject::GO_MORBOMB || proj->type == GameObject::GO_BIGMORBOMB) {
			if (proj->nearest != NULL) {

				Vector3 startScale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);
				Vector3 maxScale = Vector3(m_gridSize, m_gridSize, m_gridSize); // at arc peak
				if (proj->type == GameObject::GO_BIGMORBOMB) {
					startScale *= 1.2;
					maxScale *= 1.2;
				}
				float totalDist = (proj->target - proj->startPos).Length();
				float travelledDist = (proj->pos - proj->startPos).Length();
				float t = Math::Clamp(travelledDist / totalDist, 0.f, 1.f);
				float arc = 1.f - (2.f * t - 1.f) * (2.f * t - 1.f); // parabolic scaling shape
				proj->scale = startScale + (maxScale - startScale) * arc;

				float distance = (proj->target - proj->pos).Length();
				if (distance <= 0.001)
				{
					proj->health = 0;
					proj->active = false;
					MessageWRU msgAreaDamage = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 2.5f);
					if (proj->type == GameObject::GO_BIGMORBOMB) {
						msgAreaDamage = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 2.5f);
					}
					else {
						msgAreaDamage = MessageWRU(proj, MessageWRU::SEARCH_TYPE::NEAREST_ENEMY_INAREA, 1.5f);
					}
					Handle(&msgAreaDamage);
					for (auto& target : proj->hits)
					{
						target->health -= 15;
						target->lastAttacker = proj->owner;
					}

					proj->type = GameObject::GO_NONE;
				}
			}
		}
		//std::cout << proj->target << std::endl;
	}
	for (int i = 0; i < m_goList.size(); i++) {
		if (!m_goList[i]->active) continue;
		if (m_goList[i]->type == GameObject::GO_ATTACKER)
			std::cout << "2 " << m_goList[i]->health << std::endl;
	}
	static constexpr float TURN_TIME = 0.5f;
	static float timer = 0.f;
	// 6.	SceneTurn::Update - add code to support a turn-based system. Read this pseudo codes and try to implement on your own. The solution will be provided during lesson time.
	//remember to declare and initialize m_turn
	timer += m_speed * dt;
	if (timer > 1.0f)
	{
		timer = 0.f;

		if (m_activeSide == GameObject::SIDE_RED) ++m_turn;

		GameObject::SIDE sideThisTurn = m_activeSide;

		// ---- PHASE 1: PRE-REVEAL (sense before moving) ----
		for (GameObject* go : m_goList)
		{
			if (!go->active) continue;
			if (go->side != sideThisTurn) continue;

			RevealAround(go, go->viewRange);

			// target pick (with stickiness recommended)
			if (!go->atkTarget && !go->visibleTargets.empty())
				go->atkTarget = PickClosestVisibleTarget(go);
		}

		// ---- MOVE PHASE (only this side) ----
		for (GameObject* go : m_goList)
		{
			if (!go->active) continue;
			if (go->side != sideThisTurn) continue;
			go->currMoves = go->useMoves;
			switch (go->type)
			{
			case GameObject::GO_SCOUT:
				DFSOnce(go);
				break;
			case GameObject::GO_ATTACKER:
				std::cout << go->health << std::endl;
				if (go->sm->GetCurrentState() == "Healthy")
				{
					if (go->atkTarget == NULL) {
						DFSOnce(go, go->currMoves);
					}
					else
					{
						//attacking
						if (IsInAtkRange(go, go->atkTarget))
						{
							while (go->currMoves > 0) {
								go->atkTarget->health -= 20;
								go->currMoves -= 1;
							}
							std::cout << "attacking" << std::endl;
						}
						else {
							PathFind(go, go->atkTarget->curr, go->currMoves, go->atkRange);
							std::cout << "too far" << std::endl;
						}
					}
					if (go->health <= 70)
					{
						go->sm->SetNextState("Flee");
					}
				}
				if (go->sm->GetCurrentState() == "Flee")
				{
					MessageWRU nearestSpawner = MessageWRU(go, MessageWRU::SEARCH_TYPE::NEAREST_SPAWNER, 200);
					Handle(&nearestSpawner);
					PathFind(go, go->nearest->currNodes[0], go->currMoves, 1);
				}
				break;
			}
		}

		// ---- REVEAL PHASE (only this side) ----
		for (GameObject* go : m_goList)
		{
			if (!go->active) continue;
			if (go->side != sideThisTurn) continue;

			RevealAround(go, go->viewRange);
			if (go->visibleTargets.size() > 0)
				go->atkTarget = PickClosestVisibleTarget(go);
		}

		// ---- flip to the other side for next tick ----
		m_activeSide = (m_activeSide == GameObject::SIDE_BLUE)
			? GameObject::SIDE_RED
			: GameObject::SIDE_BLUE;
	}

	//check for death buildings and units
	{
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active)
				continue;
			if (go->health <= 0) {
				//go->sm->SetNextState("Death");

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

				--m_objectCount;
			}
		}
	}

	//Movement Section
	//for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	//{
	//	GameObject* go = (GameObject*)*it;
	//	if (!go->active)
	//		continue;
	//	Vector3 dir = go->target - go->pos;

	{
		//	//use additive speed so i can add on whenever i want
		//	go->finalMoveSpeed = (go->moveSpeed * 5) + go->supportSpeed;
		//	//if (go->type == GameObject::GO_RANGED) { std::cout << dir.Length() << std::endl; }
		//	if (go->moving == true) {
		//		if (dir.Length() < go->finalMoveSpeed * dt * m_speed)
		//		{
		//			//GO->pos reach target
		//			go->pos = go->target;
		//			
		//			if (go->moveRight)
		//				go->target = go->pos + Vector3(m_gridSize, 0, 0);

		//			else if (go->moveLeft)
		//				go->target = go->pos + Vector3(-m_gridSize, 0, 0);

		//			else if (go->moveUp)
		//				go->target = go->pos + Vector3(0, m_gridSize, 0);

		//			else if (go->moveDown)
		//				go->target = go->pos + Vector3(0, -m_gridSize, 0);

		//			else
		//				go->target = go->pos; // no movement allowed

		//			if (go->target.x < 0 || go->target.x > m_noGrid * m_gridSize || go->target.y < 0 || go->target.y > m_noGrid * m_gridSize)
		//				go->target = go->pos;
		//		}
		//		else
		//		{
		//			try
		//			{
		//				dir.Normalize();
		//				go->pos += dir * go->finalMoveSpeed * static_cast<float>(dt) * m_speed;
		//			}
		//			catch (DivideByZero)
		//			{
		//			}
		//		}
		//	}
		//}
		//for (std::vector<GameObject*>::iterator it = m_projList.begin(); it != m_projList.end(); ++it)
		//{
		//	GameObject* go = (GameObject*)*it;
		//	if (!go->active)
		//		continue;
		//	Vector3 dir = go->target - go->pos;
		//	if (dir.Length() < go->moveSpeed * dt * m_speed)
		//	{
		//		//GO->pos reach target
		//		go->pos = go->target;
		//	}
		//	else
		//	{
		//		try
		//		{
		//			dir.Normalize();
		//			go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
		//		}
		//		catch (DivideByZero)
		//		{
		//		}
		//	}
		//}
	}

	//Counting objects
	CountingGO();
	timeCounter += dt;
	
	gameStateString = GameState();
	if (gameStateString == "Red Side Wins!" || gameStateString == "Blue Side Wins!")
	{
		gamePlaying = false;
	}
}

std::string GetTimeString(float timeCounter)
{
	int totalSeconds = static_cast<int>(timeCounter);

	// clamp between 0 and 5 minutes
	if (totalSeconds < 0)
		totalSeconds = 0;

	int minutes = totalSeconds / 60;
	int seconds = totalSeconds % 60;

	std::string result = std::to_string(minutes);
	result += ":";

	// manual zero-padding for seconds
	if (seconds < 10)
		result += "0";

	result += std::to_string(seconds);

	return result; 
}

void SceneMovement_Week03::CountingGO()
{
	// ---------- ATTACKER ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_ATTACKER, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_ATTACKER, GameObject::SIDE_RED);

		int blueAttacker = HandleCount(&msgBlue);
		int redAttacker = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_ATTACKER] = blueAttacker;
		m_numRedGO[GameObject::GO_ATTACKER] = redAttacker;
		m_numGO[GameObject::GO_ATTACKER] = blueAttacker + redAttacker;
	}

	// ---------- RANGED ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_RANGED, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_RANGED, GameObject::SIDE_RED);

		int blueRanged = HandleCount(&msgBlue);
		int redRanged = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_RANGED] = blueRanged;
		m_numRedGO[GameObject::GO_RANGED] = redRanged;
		m_numGO[GameObject::GO_RANGED] = blueRanged + redRanged;
	}

	// ---------- SUPPORT ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_SUPPORT, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_SUPPORT, GameObject::SIDE_RED);

		int blueSupport = HandleCount(&msgBlue);
		int redSupport = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_SUPPORT] = blueSupport;
		m_numRedGO[GameObject::GO_SUPPORT] = redSupport;
		m_numGO[GameObject::GO_SUPPORT] = blueSupport + redSupport;
	}

	// ---------- MECHANIC ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_MECHANIC, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_MECHANIC, GameObject::SIDE_RED);

		int blueMech = HandleCount(&msgBlue);
		int redMech = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_MECHANIC] = blueMech;
		m_numRedGO[GameObject::GO_MECHANIC] = redMech;
		m_numGO[GameObject::GO_MECHANIC] = blueMech + redMech;
	}

	// ---------- TANK ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_TANK, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_TANK, GameObject::SIDE_RED);

		int blueTank = HandleCount(&msgBlue);
		int redTank = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_TANK] = blueTank;
		m_numRedGO[GameObject::GO_TANK] = redTank;
		m_numGO[GameObject::GO_TANK] = blueTank + redTank;
	}

	// ---------- MORTAR ----------
	{
		MessageHowManyUnit msgBlue(GameObject::GO_MORTAR, GameObject::SIDE_BLUE);
		MessageHowManyUnit msgRed(GameObject::GO_MORTAR, GameObject::SIDE_RED);

		int blueMortar = HandleCount(&msgBlue);
		int redMortar = HandleCount(&msgRed);

		m_numBlueGO[GameObject::GO_MORTAR] = blueMortar;
		m_numRedGO[GameObject::GO_MORTAR] = redMortar;
		m_numGO[GameObject::GO_MORTAR] = blueMortar + redMortar;
	}
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
	float offsetY = 3.5f;
	modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x + offsetX,
		m_gridOffset + m_gridSize * go->curr.y - offsetY,
		6);
	modelStack.Scale(barWidth * healthRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_CUBE], false);
	modelStack.PopMatrix();

	// draw the background (max health)
	modelStack.PushMatrix();
	modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x + offsetX,
		m_gridOffset + m_gridSize * go->curr.y - offsetY,
		6);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXCUBE], false);
	modelStack.PopMatrix();


	modelStack.PushMatrix();
	// shift left edge fixed: move half the reduced width to the left
	float energyoffsetX = -(barWidth * (1.f - energyRatio));
	offsetY = 4.9f;
	modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x + offsetX,
		m_gridOffset + m_gridSize * go->curr.y - offsetY,
		6);
	modelStack.Scale(barWidth * energyRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_ENERGYCUBE], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x + offsetX,
		m_gridOffset + m_gridSize * go->curr.y - offsetY,
		6);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXENERGYCUBE], false);
	modelStack.PopMatrix();
}

void SceneMovement_Week03::RenderGOBar(GameObject* go, float vertScale, Vector3 pos)
{
	float healthRatio = go->health / go->maxHealth;
	float energyRatio = go->energy / go->maxEnergy;
	float barWidth = go->scale.x / 1.8f;
	float barHeight = go->scale.y / vertScale;

	// draw the health (shrinks left -> right)
	modelStack.PushMatrix();
	// shift left edge fixed: move half the reduced width to the left
	float offsetX = -(barWidth * (1.f - healthRatio));
	modelStack.Translate(pos.x + offsetX, pos.y - 3.5f, zOffset);
	modelStack.Scale(barWidth * healthRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_CUBE], false);
	modelStack.PopMatrix();

	// draw the background (max health)
	modelStack.PushMatrix();
	modelStack.Translate(pos.x, pos.y - 3.5f, zOffset);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXCUBE], false);
	modelStack.PopMatrix();


	modelStack.PushMatrix();
	// shift left edge fixed: move half the reduced width to the left
	float energyoffsetX = -(barWidth * (1.f - energyRatio));
	modelStack.Translate(pos.x + energyoffsetX, pos.y - 4.9, zOffset);
	modelStack.Scale(barWidth * energyRatio, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_ENERGYCUBE], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(pos.x, pos.y - 4.9, zOffset);
	modelStack.Scale(barWidth, barHeight, go->scale.z);
	RenderMesh(meshList[GEO_MAXENERGYCUBE], false);
	modelStack.PopMatrix();
}

void SceneMovement_Week03::RenderGO(GameObject* go)
{
	std::ostringstream ss;
	switch (go->type)
	{
	case GameObject::GO_NPC: //Render GO_NPC
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x,
			m_gridOffset + m_gridSize *
			go->curr.y,
			6);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
		RenderMesh(meshList[GEO_AGENT], false);
		modelStack.PopMatrix();
	}
	break;
	case GameObject::GO_MAINBASE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_BASEBLUE], false);
		else
			RenderMesh(meshList[GEO_BASERED], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);

		modelStack.PushMatrix();
		if (go->side == GameObject::SIDE_RED)
		{
			modelStack.Translate(132.5, 90, zOffset);
			RenderGOBar(go, 14, Vector3(0, -1.5, 0));
		}
		else
		{
			modelStack.Translate(132.5, 50, zOffset);
			RenderGOBar(go, 14, Vector3(0, -1.5, 0));
		}
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_BASEBLUE], false);
		else
			RenderMesh(meshList[GEO_BASERED], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);
		break;
	case GameObject::GO_SPAWNER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_SPAWNERBLUE], false);
		else
			RenderMesh(meshList[GEO_SPAWNERRED], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(7.5 * 2, 7.5 * 2, 1); // radius (scale by 2 for diameter)
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(25 * 2, 25 * 2, 1); // radius (scale by 2 for diameter)
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);


		modelStack.PushMatrix();
		if (go->side == GameObject::SIDE_RED)
		{
			if (go->pos.y < m_worldHeight / 2)
			{
				modelStack.Translate(go->pos.x + 125, go->pos.y + 60, zOffset);
			}
			else {
				modelStack.Translate(go->pos.x + 90, go->pos.y + 20, zOffset);
			}
			RenderGOBar(go, 14, Vector3(0, -1.5, 0));

		}
		else
		{
			if (go->pos.y < m_worldHeight / 2)
			{
				modelStack.Translate(go->pos.x + 75, go->pos.y + 20, zOffset);
			}
			else {
				modelStack.Translate(go->pos.x + 40, go->pos.y - 20, zOffset);
			}
			RenderGOBar(go, 14, Vector3(0, -1.5, 0));
		}

		ss.str("");
		ss.clear();
		ss << "W:" << go->woodenLogs << " M:" << go->metalParts;

		modelStack.PushMatrix();
		modelStack.Translate(-4.5, 6, zOffset);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->side == GameObject::SIDE_RED)
			RenderMesh(meshList[GEO_SPAWNERBLUE], false);
		else
			RenderMesh(meshList[GEO_SPAWNERRED], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 14);
		break;
	case GameObject::GO_ATTACKER:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Dead") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;

		if (state == "Flee")
			geo = isBlue ? GEO_ATTACKER1_FLEEING : GEO_ATTACKER2_FLEEING;
		else if (state == "StayStrong")
			geo = isBlue ? GEO_ATTACKER1_STAY_STRONG : GEO_ATTACKER2_STAY_STRONG;
		else if (state == "NearDeath")
			geo = isBlue ? GEO_ATTACKER1_NEAR_DEATH : GEO_ATTACKER2_NEAR_DEATH;
		else if (state == "Helping")
			geo = isBlue ? GEO_ATTACKER1_HELPING : GEO_ATTACKER2_HELPING;
		else
			geo = isBlue ? GEO_ATTACKER1_HEALTHY : GEO_ATTACKER2_HEALTHY;

		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x,
			m_gridOffset + m_gridSize *
			go->curr.y,
			6);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
	case GameObject::GO_RANGED:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;

		if (state == "NearDeath")
			geo = isBlue ? GEO_RANGE1_NEAR_DEATH : GEO_RANGE2_NEAR_DEATH;
		else if (state == "Panic")
			geo = isBlue ? GEO_RANGE1_PANIC : GEO_RANGE2_PANIC;
		else if (state == "Hurt")
			geo = isBlue ? GEO_RANGE1_HURT : GEO_RANGE2_HURT;
		else
			geo = isBlue ? GEO_RANGE1_HEALTHY : GEO_RANGE2_HEALTHY;

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
	case GameObject::GO_SUPPORT:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;

		if (state == "UrgentHealing")
			geo = isBlue ? GEO_SUPPORT1_URGENT_HEALING : GEO_SUPPORT2_URGENT_HEALING;
		else if (state == "Healing")
			geo = isBlue ? GEO_SUPPORT1_HEALING : GEO_SUPPORT2_HEALING;
		else if (state == "Hiding")
			geo = isBlue ? GEO_SUPPORT1_HIDING : GEO_SUPPORT2_HIDING;
		else if (state == "Hurt")
			geo = isBlue ? GEO_SUPPORT1_HURT : GEO_SUPPORT2_HURT;
		else
			geo = isBlue ? GEO_SUPPORT1_HEALTHY : GEO_SUPPORT2_HEALTHY;

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
	case GameObject::GO_PROJECTILE:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;
		geo = isBlue ? GEO_BULLET1 : GEO_BULLET2;
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObject::GO_ROCKETS:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;
		geo = isBlue ? GEO_BULLET1 : GEO_BULLET2;
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObject::GO_MORBOMB:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;
		geo = isBlue ? GEO_BULLET1 : GEO_BULLET2;
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObject::GO_BIGMORBOMB:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		//modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;
		geo = isBlue ? GEO_BULLET1 : GEO_BULLET2;
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObject::GO_MECHANIC:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo;

		if (state == "GoldenChase")
			geo = isBlue ? GEO_MECHANIC1_GOLD : GEO_MECHANIC2_GOLD;
		else if (state == "Building")
			geo = isBlue ? GEO_MECHANIC1_BUILDING : GEO_MECHANIC2_BUILDING;
		else if (state == "Hiding")
			geo = isBlue ? GEO_MECHANIC1_HIDING : GEO_MECHANIC2_HIDING;
		else if (state == "Hurt")
			geo = isBlue ? GEO_MECHANIC1_HURT : GEO_MECHANIC2_HURT;
		else
			geo = isBlue ? GEO_MECHANIC1_HEALTHY : GEO_MECHANIC2_HEALTHY;

		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x,
			m_gridOffset + m_gridSize *
			go->curr.y,
			6);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
	case GameObject::GO_TANK:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo = GEO_TANK1_HEALTHY;

		if (state == "Suicide")
			geo = isBlue ? GEO_TANK1_BOMB : GEO_TANK2_BOMB;
		else if (go->panicking)
			geo = isBlue ? GEO_TANK1_PANIC : GEO_TANK2_PANIC;
		else
			geo = isBlue ? GEO_TANK1_HEALTHY : GEO_TANK2_HEALTHY;

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
	case GameObject::GO_MORTAR:
	{
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo = isBlue ? GEO_MORTAR1_HEALTHY : GEO_MORTAR2_HEALTHY;

		if (state == "Panic")
			geo = isBlue ? GEO_MORTAR1_PANIC : GEO_MORTAR2_PANIC;

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;


		//modelStack.PushMatrix();
		//modelStack.Translate(go->pos.x, go->pos.y, zOffset + 0.001f); // small offset to prevent z-fighting
		//modelStack.Scale(20 * 2, 20 * 2, 1); // radius (scale by 2 for diameter)
		//RenderMesh(meshList[GEO_CIRCLE], false);
		//modelStack.PopMatrix();

		//modelStack.PushMatrix();
		//modelStack.Translate(go->pos.x, go->pos.y, zOffset + 0.001f); // small offset to prevent z-fighting
		//modelStack.Scale(70 * 2, 70 * 2, 1); // radius (scale by 2 for diameter)
		//RenderMesh(meshList[GEO_CIRCLE], false);
		//modelStack.PopMatrix();
		//break;
	}
	case GameObject::GO_GOLDENORB:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_GOLDENORB], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	case GameObject::GO_SCOUT:
		std::string state = go->sm->GetCurrentState();
		if (state == "Death") break;

		bool isBlue = (go->side == GameObject::SIDE_BLUE);
		GEOMETRY_TYPE geo = isBlue ? GEO_MORTAR1_HEALTHY : GEO_MORTAR2_HEALTHY;

		if (state == "Panic")
			geo = isBlue ? GEO_MORTAR1_PANIC : GEO_MORTAR2_PANIC;

		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + m_gridSize * go->curr.x,
			m_gridOffset + m_gridSize *
			go->curr.y,
			6);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();
		RenderGOBar(go, 7);
		break;
	}
}

int SceneMovement_Week03::HandleCount(Message* message)
{
	MessageHowManyUnit* msgHowMany = dynamic_cast<MessageHowManyUnit*>(message);
	if (msgHowMany) {
		GameObject::GAMEOBJECT_TYPE type = msgHowMany->go_type;
		int count = 0;
		for (GameObject* go2 : m_goList)
		{
			if (!go2->active)
				continue;
			if(go2->type == type && go2->side == msgHowMany->side)
			{
				count++;
			}
		}
		return count;
	}
	return -1;
}

bool SceneMovement_Week03::Handle(Message* message)
{
	MessageNewTile* msgNewTile = dynamic_cast<MessageNewTile*>(message);
	if (msgNewTile)
	{
		Maze::TILE_CONTENT t = msgNewTile->tile;
		switch (t)
		{
		case Maze::TILE_ORE:
			std::cout << "Call for Mechanics" << std::endl;
			//call for closest 2 mechanics
			//mechanics check if they are already assigned to a ore
			//if yes, check if this ore is closer
			//else check all other mechanics
			break;
		}
	}

	MessageRevealUnit* msgRevealUnit = dynamic_cast<MessageRevealUnit*>(message);
	if (msgRevealUnit) {
		if (msgRevealUnit->target->type == GameObject::GO_ATTACKER) {
			std::cout << "Attacker Revealed!" << std::endl;
			if (msgRevealUnit->spotter->type == GameObject::GO_ATTACKER)
				msgRevealUnit->spotter->atkTarget = msgRevealUnit->target;
		}
	}

	MessageMechanicBuild* msgMechBuild = dynamic_cast<MessageMechanicBuild*>(message);
	if (msgMechBuild) {
		for (GameObject* go2 : m_goList)
		{
			if (!go2->active)
				continue;
			if (go2->type != GameObject::GO_MECHANIC)
				continue;
			if (go2->sm->GetCurrentState() != "Healthy" && go2->sm->GetCurrentState() != "Hurt")
				continue;
			if (go2->nearest != msgMechBuild->go)
				continue;
			if (go2->side != msgMechBuild->go->side)
				continue;
			go2->sm->SetNextState("Building");
			return true;
		}
	}

	MessageSpawnProj* msgSpawnProj = dynamic_cast<MessageSpawnProj*>(message);
	if (msgSpawnProj) {

		GameObject* projectile = FetchProj(GameObject::GO_PROJECTILE);
		projectile->type = GameObject::GO_PROJECTILE;
		projectile->pos = msgSpawnProj->go->pos;
		projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);

		Vector3 dir = (msgSpawnProj->go->nearest->pos - msgSpawnProj->go->pos).Normalized();
		float overshootDistance = 100.0f;
		projectile->target = msgSpawnProj->go->nearest->pos + dir * overshootDistance;
		projectile->nearest = msgSpawnProj->go->nearest; //use nearest as proj target object
		projectile->moveSpeed = 20.f;
		projectile->side = msgSpawnProj->go->side;
		projectile->owner = msgSpawnProj->go;
	}

	MessageSpawnMorBomb* msgSpawnMorBomb = dynamic_cast<MessageSpawnMorBomb*>(message);
	if (msgSpawnMorBomb) {
		GameObject* projectile = FetchProj(GameObject::GO_MORBOMB);
		projectile->type = GameObject::GO_MORBOMB;
		projectile->pos = msgSpawnMorBomb->go->pos;
		projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);

		projectile->target = msgSpawnMorBomb->go->nearest->pos;//use nearest as proj target object
		projectile->nearest = msgSpawnMorBomb->go->nearest;
		projectile->moveSpeed = 15.f;
		projectile->side = msgSpawnMorBomb->go->side;
		projectile->startPos = projectile->pos;
		projectile->owner = msgSpawnMorBomb->go;
	}

	MessageSpawnBigMorBomb* msgSpawnBigMorBomb = dynamic_cast<MessageSpawnBigMorBomb*>(message);
	if (msgSpawnBigMorBomb) {
		GameObject* projectile = FetchProj(GameObject::GO_BIGMORBOMB);
		projectile->type = GameObject::GO_BIGMORBOMB;
		projectile->pos = msgSpawnBigMorBomb->go->pos;
		projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);
		projectile->scale *= 1.2f;

		projectile->target = msgSpawnBigMorBomb->go->nearest->pos;//use nearest as proj target object
		projectile->nearest = msgSpawnBigMorBomb->go->nearest;
		projectile->moveSpeed = 15.f;
		projectile->side = msgSpawnBigMorBomb->go->side;
		projectile->startPos = projectile->pos;
		projectile->owner = msgSpawnBigMorBomb->go;
	}

	MessageSpawnProjTank* msgSpawnProj2 = dynamic_cast<MessageSpawnProjTank*>(message);
	if (msgSpawnProj2) {
		GameObject* projectile = NULL;
		if (msgSpawnProj2->rocketOrProj) {
			projectile = FetchProj(GameObject::GO_ROCKETS);
			projectile->type = GameObject::GO_ROCKETS;
			//std::cout << "Spawning Rocket" << std::endl;
		}
		else
		{
			projectile = FetchProj(GameObject::GO_PROJECTILE);
			projectile->type = GameObject::GO_PROJECTILE;
			//std::cout << "Spawning Projectile" << std::endl;
		}
		projectile->pos = msgSpawnProj2->go->pos;
		projectile->scale = Vector3(m_gridSize / 4, m_gridSize / 4, m_gridSize / 4);
		Vector3 delta = msgSpawnProj2->target->pos - msgSpawnProj2->go->pos;

		Vector3 dir;
		float len = delta.Length();
		if (len > Math::EPSILON)   // safe to normalize
		{
			dir = delta.Normalized();
		}
		else
		{
			// fallback direction if on top of target
			dir = Vector3(1.f, 0.f, 0.f); // or keep previous dir, or skip spawning
		}
		float overshootDistance = 100.0f;
		projectile->target = msgSpawnProj2->target->pos + dir * overshootDistance;
		projectile->nearest = msgSpawnProj2->target; //use nearest as proj target object
		projectile->moveSpeed = 20.f;
		projectile->side = msgSpawnProj2->go->side;
		projectile->owner = msgSpawnProj2->go;
	}

	MessageSpawnAttacker* msgSpawnAtk = dynamic_cast<MessageSpawnAttacker*>(message);
	if (msgSpawnAtk) {
		SpawnUnit(msgSpawnAtk->go->side, msgSpawnAtk->go->pos, GameObject::GO_ATTACKER);
	}

	MessageSpawnMortar* msgSpawnMor = dynamic_cast<MessageSpawnMortar*>(message);
	if (msgSpawnMor) {
		SpawnMetalUnit(msgSpawnMor->go->side, msgSpawnMor->go->pos, GameObject::GO_MORTAR);
	}



	MessageWRU* messageWRU = dynamic_cast<MessageWRU*>(message);
	if (messageWRU)
	{
		GameObject* go = messageWRU->go;
		static const MessageWRU::SEARCH_TYPE dontResetNearest[] = {
			MessageWRU::NEAREST_FREE_SUP,
			MessageWRU::NEAREST_URG_SUP,
			MessageWRU::NEAREST_ALLY_ATTACKER,
			MessageWRU::NEAREST_MECHANIC_HEAL,
			MessageWRU::ALLYACTIVECOUNT,
			MessageWRU::ATTACKERCOUNT,
			MessageWRU::NEAREST_GOLDENORB,
		};
		bool keepNearest = false;
		// Loop through the array
		for (int i = 0; i < sizeof(dontResetNearest) / sizeof(dontResetNearest[0]); i++)
		{
			if (messageWRU->type == dontResetNearest[i])
			{
				keepNearest = true;    // found match → we should NOT reset
				break;                 // stop the loop
			}
		}
		// If message type is NOT in the list → reset nearest
		if (!keepNearest)
		{
			go->nearest = nullptr;
		}

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
			else if (messageWRU->type == MessageWRU::NEAREST_GOLDENORB && go2->type == GameObject::GO_GOLDENORB)
			{
				go->specialTarget = go2;
			}
			else if (messageWRU->type == MessageWRU::ATTACKERCOUNT && go2->side != go->side)
			{
				if (go2->nearest == go->nearest)
				{
					go->steps++;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_OBJ && go->side != go2->side)
			{
				if(go2->type == GameObject::GO_SPAWNER || go2->type == GameObject::GO_MAINBASE)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < messageWRU->threshold && distance < nearestDistance)
					{
						nearestDistance = distance;
						go->nearest = go2;
					}
				}
			}
			else if (messageWRU->type == MessageWRU::ALLYACTIVECOUNT && go2->side == go->side)
			{
				if (go2->type == GameObject::GO_SPAWNER || go2->type == GameObject::GO_MAINBASE || go2->type == GameObject::GO_GOLDENORB)
					continue;

				if (go2->hiding)
					continue;

				if (go2 == go)
					continue;
				
				go->alliesActiveCount++;
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ENEMY_INAREA && go2->side != go->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < m_gridSize * messageWRU->threshold)
				{
					go->hits.push_back(go2);
				}
				
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ALLY_ATTACKER && go2->type == GameObject::GO_ATTACKER && go2->side == go->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					bool alreadyHelping = false;
					if (go2->sm->GetCurrentState() != "Healthy")
						alreadyHelping = true;

					if (go2->nearest == go->lastAttacker) //someone is already attacking my attacker
						alreadyHelping = true;

					nearestDistance = distance;
					go->external2 = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_URG_SUP && go2->type == GameObject::GO_SUPPORT && go2->side == go->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					bool alreadyHelping = false;
					bool busyUrgent = false;

					if (go2->sm->GetCurrentState() == "UrgentHealing")
						busyUrgent = true;
					if (go2->hiding == true) continue;
					if (go2->sm->GetCurrentState() != "Healthy") continue;

					// We cannot steal an urgent healer
					// also ensures if this unit is already being healed, it wont find another healer
					if (busyUrgent)
						continue;

					if (!alreadyHelping)
					{
						if (distance < nearestDistance)
						{
							nearestDistance = distance;
							go->external = go2;
							go->urgent = true;
						}
					}
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_FREE_SUP && go2->type == GameObject::GO_SUPPORT && go2->side == go->side)
			{
				int bestPriority = -999;

				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					if (go2->hiding == true) continue;
					bool alreadyHealing = false;
					for (auto go3 : m_goList)
					{
						if (!go3->active) continue;
						if (go3->type != GameObject::GO_SUPPORT) continue;
						if (go3->sm->GetCurrentState() != "Healthy") continue;
						if (go3->sm->GetCurrentState() != "Healing") continue;

						if (go3->nearest == go)
						{
							alreadyHealing = true;
							break;
						} 
					}

					if (!alreadyHealing)
					{
						int pri = GetHealPriority(go);     
						if (pri > bestPriority || (pri == bestPriority && distance < nearestDistance)) // tiebreaker
						{
							bestPriority = pri;
							nearestDistance = distance;
							go->external = go2;
						}
					}
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_MECHANIC_HEAL && go2->type == GameObject::GO_MECHANIC && go2->side == go->side)
			{
				int bestPriority = -999;

				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					if (go2->hiding == true) continue;
					bool alreadyHealing = false;
					for (auto go3 : m_goList)
					{
						if (!go3->active) continue;
						if (go3->type != GameObject::GO_MECHANIC) continue;
						if (go3->sm->GetCurrentState() != "Healthy") continue;
						if (go3->sm->GetCurrentState() != "Healing") continue;

						if (go3->nearest == go)
						{
							alreadyHealing = true;
							break;
						}
					}
					if (!alreadyHealing)
					{
						int pri = GetHealPriority(go);    
						if (pri > bestPriority || (pri == bestPriority && distance < nearestDistance)) // tiebreaker
						{
							bestPriority = pri;
							nearestDistance = distance;
							go->external = go2;
						}
					}
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_MORTAR_ENEMY && go2->side != go->side)
			{
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance > messageWRU->mortarTooCloseValue && distance < nearestDistance)
				{
					if (go2->type == GameObject::GO_SPAWNMORTARAREA) continue;
					if (go2->type == GameObject::GO_GOLDENORB) continue;
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::FURTHEST_FRONTLINE && go2->side == go->side)
			{
				if (go2->type != GameObject::GO_SPAWNER && go2->type != GameObject::GO_MAINBASE && go2->type != GameObject::GO_GOLDENORB && go2->type != GameObject::GO_TANK && go2->type != GameObject::GO_SPAWNMORTARAREA) {

					float distance = (go->pos - go2->pos).Length();
					if (distance > messageWRU->threshold)
						continue;

					// side start positions (replace 0.f / 600.f with your actual values/constants)
					float RED_START_X = 0.0f;
					float BLUE_START_X = 600.0f;

					float frontValue = 0.0f;

					if (go->side == GameObject::SIDE_BLUE)
					{
						// how far BLUE has moved from 600 towards 0 (positive when they push left)
						frontValue = BLUE_START_X - go2->pos.x;
					}
					else // assume RED
					{
						// how far RED has moved from 0 towards +X
						frontValue = go2->pos.x - RED_START_X;
					}

					// Optional: if somehow behind their own start line, ignore
					if (frontValue <= 0.0f)
						continue;

					if (frontValue > highestEnergy)
					{
						highestEnergy = frontValue;
						go->nearest = go2;   // furthest advanced ally on this side
					}
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ENEMY && go2->side != go->side)
			{
				if (go2->type == GameObject::GO_SPAWNMORTARAREA) continue;
				if (go2->type == GameObject::GO_GOLDENORB) continue;
				float distance = (go->pos - go2->pos).Length();
				if (distance < messageWRU->threshold && distance < nearestDistance)
				{
					if (go2->hiding == true) continue;
					nearestDistance = distance;
					go->nearest = go2;
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_ALLY_NOSUP && go2->side == go->side)
			{
				if (go2 != go) {
					if (go2->type != GameObject::GO_SPAWNER && go2->type != GameObject::GO_MAINBASE && go2->type != GameObject::GO_GOLDENORB && go2->type != GameObject::GO_SPAWNMORTARAREA && go2->type != GameObject::GO_SUPPORT) {
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

void SceneMovement_Week03::RenderDebugGO(GameObject::SIDE side, float y)
{
	bool isBlue = (side == GameObject::SIDE_BLUE);

	float spacing = m_gridSize * 1.5f;   // distance between icons
	float x = 112.5;                     // starting X position
	float iconSize = m_gridSize;         // size of each icon
	float z = 5.0f;                      // draw in front of BG

	GEOMETRY_TYPE geo;
	std::ostringstream ss;

	// small vertical offset for text under the icon
	float textOffsetY = iconSize * 0.8f;

	// 1) ATTACKER
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_ATTACKER1_HEALTHY : GEO_ATTACKER2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		// number under icon
		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_ATTACKER]
			: m_numRedGO[GameObject::GO_ATTACKER]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		x += spacing;
	}

	// 2) RANGED
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_RANGE1_HEALTHY : GEO_RANGE2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_RANGED]
			: m_numRedGO[GameObject::GO_RANGED]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		x += spacing;
	}

	// 3) SUPPORT
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_SUPPORT1_HEALTHY : GEO_SUPPORT2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_SUPPORT]
			: m_numRedGO[GameObject::GO_SUPPORT]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		x += spacing;
	}

	// 4) MECHANIC
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_MECHANIC1_HEALTHY : GEO_MECHANIC2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_MECHANIC]
			: m_numRedGO[GameObject::GO_MECHANIC]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		x += spacing;
	}

	// 5) TANK
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_TANK1_HEALTHY : GEO_TANK2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_TANK]
			: m_numRedGO[GameObject::GO_TANK]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();

		x += spacing;
	}

	// 6) MORTAR
	{
		float iconX = x + 1.25f;

		geo = isBlue ? GEO_MORTAR1_HEALTHY : GEO_MORTAR2_HEALTHY;
		modelStack.PushMatrix();
		modelStack.Translate(x, y, z);
		modelStack.Rotate(180, 0, 0, 1);
		modelStack.Scale(iconSize, iconSize, iconSize);
		RenderMesh(meshList[geo], false);
		modelStack.PopMatrix();

		ss.str("");
		ss.clear();
		ss << (isBlue
			? m_numBlueGO[GameObject::GO_MORTAR]
			: m_numRedGO[GameObject::GO_MORTAR]);

		modelStack.PushMatrix();
		modelStack.Translate(iconX, y - textOffsetY, z);
		modelStack.Scale(5, 5, 5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0));
		modelStack.PopMatrix();
	}
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
	RenderDebugGO(GameObject::SIDE_BLUE, 35);
	RenderDebugGO(GameObject::SIDE_RED,75);
	
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * col, m_gridOffset + m_gridSize * row, 5);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			// Exercise Week 08
			// b.Render the tiles using m_maze.m_grid instead of m_myGrid
			switch (m_myGrid[row * m_noGrid + col])
			{
			case Maze::TILE_WALL:
				RenderMesh(meshList[GEO_WALL], false);
				break;
			case Maze::TILE_FOG:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;
			case Maze::TILE_ORE:
				meshList[GEO_ORE]->material.kAmbient.Set(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_ORE], true);
				break;
			case Maze::TILE_EMPTY:
				//RenderMesh(meshList[GEO_FLOOR], false);
				break;
			}
			modelStack.PopMatrix();
		}
	}

	float miniSize = 2.0f;                 // matches your Scale(2,2,2)
	float miniOffset = m_gridOffset;        // or pick a different corner offset
	float step = miniSize;                 // 1 tile = 1 step (no gaps)

	// If you want a small gap between tiles, use:
	// float step = miniSize + 0.2f;

	int i = 0;
	for (auto b : b_grid)
	{
		int cellX = i % m_noGrid;
		int cellY = i / m_noGrid;

		modelStack.PushMatrix();
		modelStack.Translate(
			100 + miniOffset + step * cellX,
			miniOffset + step * cellY,
			10.0f
		);
		modelStack.Scale(miniSize, miniSize, miniSize);
		if (b)
		{			
			switch (b_grid[cellY * m_noGrid + cellX])
			{
			case Maze::TILE_WALL:
				RenderMesh(meshList[GEO_WALL], false);
				break;
			case Maze::TILE_FOG:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;
			case Maze::TILE_ORE:
				meshList[GEO_ORE]->material.kAmbient.Set(1.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_ORE], true);
				break;
			case Maze::TILE_EMPTY:
				RenderMesh(meshList[GEO_FLOOR], false);
				break;
			}
		}
		else
		{
			meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0, 0, 0);
			RenderMesh(meshList[GEO_WHITEQUAD], true);
		}

		modelStack.PopMatrix();
		++i;
	}
	
	//On screen text

	std::ostringstream ss;
	ss << "Match In Progress";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 49, 10);

	ss.str("");
	ss << gameStateString;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 49, 7.5);

	ss.str("");
	ss << GetTimeString(timeCounter);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 49, 5);

	std::string input;
	if (!gamePlaying)
	{
		if (timeCounter < 1) { input = "Press Space To Start"; }
		else { input = "Press Escape To End"; }
	}
	ss.str("");
	ss << input;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 49, 2.5);

	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Turn:" << m_turn;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn Maze " << m_mazeKey;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);

}

std::string SceneMovement_Week03::GameState()
{
	GameObject* m_mainBaseBlue = NULL;
	GameObject* m_mainBaseRed = NULL;

	GameObject* m_blueSpawners[2] = { NULL, NULL };
	GameObject* m_redSpawners[2] = { NULL, NULL };

	//finding objectives
	{
		int blueIndex = 0;
		int redIndex = 0;

		for (int i = 0; i < (int)m_goList.size(); i++)
		{
			GameObject* go = m_goList[i];
			if (!go)
				continue;
			if (!go->active)
				continue;

			// --- MAIN BASES ---
			if (go->type == GameObject::GO_MAINBASE)
			{
				if (go->side == GameObject::SIDE_BLUE)
					m_mainBaseBlue = go;
				else if (go->side == GameObject::SIDE_RED)
					m_mainBaseRed = go;
			}
			// --- SPAWNERS ---
			else if (go->type == GameObject::GO_SPAWNER)
			{
				if (go->side == GameObject::SIDE_BLUE)
				{
					if (blueIndex < 2)
					{
						m_blueSpawners[blueIndex] = go;
						blueIndex++;
					}
				}
				else if (go->side == GameObject::SIDE_RED)
				{
					if (redIndex < 2)
					{
						m_redSpawners[redIndex] = go;
						redIndex++;
					}
				}
			}
		}
	}

	float redSpawnerHealth = 0.0f;
	float blueSpawnerHealth = 0.0f;

	if (m_redSpawners[0])
		redSpawnerHealth += m_redSpawners[0]->health;
	if (m_redSpawners[1])
		redSpawnerHealth += m_redSpawners[1]->health;

	if (m_blueSpawners[0])
		blueSpawnerHealth += m_blueSpawners[0]->health;
	if (m_blueSpawners[1])
		blueSpawnerHealth += m_blueSpawners[1]->health;

	float redBasePer = 0.0f;
	float blueBasePer = 0.0f;

	if (m_mainBaseRed && m_mainBaseRed->maxHealth > 0.0f)
		redBasePer = m_mainBaseRed->health / m_mainBaseRed->maxHealth;

	if (m_mainBaseBlue && m_mainBaseBlue->maxHealth > 0.0f)
		blueBasePer = m_mainBaseBlue->health / m_mainBaseBlue->maxHealth;

	//if (timeCounter >= 300)
	//{
	//	if (redBasePer == 1 && blueBasePer == 1)
	//	{
	//		if (redSpawnerHealth > blueSpawnerHealth)
	//			return "Blue Side Wins!";
	//		else if (blueSpawnerHealth > redSpawnerHealth)
	//			return "Red Side Wins!";
	//		else if (redSpawnerHealth == blueSpawnerHealth)
	//			return "The Game is Tied Currently";
	//	}
	//	else if (redBasePer > blueBasePer)
	//	{
	//		return "Blue Side Wins!";
	//	}
	//	else if (blueBasePer > redBasePer)
	//	{
	//		return "Red Side Wins!";
	//	}
	//	else if (blueBasePer == redBasePer)
	//	{
	//		return "The Game is Tied Currently";
	//	}
	//}

	if (redBasePer <= 0)
	{
		return "Red Side Wins!";
	}
	else if (blueBasePer <= 0)
	{
		return "Blue Side Wins!";
	}

	if (redBasePer == 1 && blueBasePer == 1)
	{
		if (redSpawnerHealth > blueSpawnerHealth)
			return "Blue Side is Winning";
		else if (blueSpawnerHealth > redSpawnerHealth)
			return "Red Side is Winning";
		else if (redSpawnerHealth == blueSpawnerHealth)
			return "The Game is Tied Currently";
	}
	else if (redBasePer > blueBasePer)
	{
		return "Blue Side is Winning";
	}
	else if (blueBasePer > redBasePer)
	{ 
		return "Red Side is Winning";
	}
	else if (blueBasePer == redBasePer)
	{
		return "The Game is Tied Currently";
	}
}

void SceneMovement_Week03::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_UP) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_DOWN);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = m_maze.See(next);
			}
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_DOWN) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_UP);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = m_maze.See(next);
			}
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_LEFT) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_RIGHT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = m_maze.See(next);
			}
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_RIGHT) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_LEFT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = m_maze.See(next);
			}
		}
	}
}

void SceneMovement_Week03::CarveUntilNoFog()
{
	while (true)
	{
		std::fill(m_visited.begin(), m_visited.end(), false);
		DFS(m_start);
		bool fog = false;

		for (auto t : m_myGrid)
		{
			if (t == Maze::TILE_FOG) fog = true;
		}
		if (!fog) { break; }

		bool carved = false;

		const int dx[4] = { 1,-1,0,0 };
		const int dy[4] = { 0,0,1,-1 };

		for (int y = 0; y < (int)m_noGrid && !carved; y++)
			for (int x = 0; x < (int)m_noGrid && !carved; x++)
			{
				int idx = y * m_noGrid + x;
				if (m_myGrid[idx] != Maze::TILE_FOG) continue; // we're looking at fog walls

				// if this fog tile touches ANY seen tile, break THIS tile
				for (int k = 0; k < 4; k++)
				{
					int nx = x + dx[k], ny = y + dy[k];
					if (nx < 0 || nx >= (int)m_noGrid || ny < 0 || ny >= (int)m_noGrid) continue;

					int nidx = ny * m_noGrid + nx;

					if (m_myGrid[nidx] != Maze::TILE_FOG)
					{
						//std::cout << ny << " , " << nx << std::endl;
						// carve the fog tile (idx) in the TRUE grid
						m_maze.m_grid[nidx] = Maze::TILE_EMPTY;
						m_myGrid[nidx] = Maze::TILE_EMPTY; // optional immediate reveal
						carved = true;
						break;
					}
				}
			}

		if (!carved) break; // safety
	}
}

//helper function to check if given index is within boundary
int SceneMovement_Week03::IsWithinBoundary(int x) const
{
	return x >= 0 && x < m_noGrid;
}

//helper function to convert 2d indices to 1d index
int SceneMovement_Week03::Get1DIndex(int x, int y) const
{
	return y * m_noGrid + x;
}

int SceneMovement_Week03::HeuristicManhattan(const MazePt& a, const MazePt& b)
{
	return abs(a.x - b.x) + abs(a.y - b.y) * 1;
}

bool SceneMovement_Week03::AStar(GameObject* go, MazePt start, MazePt end)
{
	const int N = m_noGrid * m_noGrid;

	go->path.clear();

	// Pick team memory ONCE
	std::vector<Maze::TILE_CONTENT>& teamGrid =
		(go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;

	const int INF = INT_MAX / 4;

	std::vector<int> gCost(N, INF);
	std::vector<int> fCost(N, INF);
	std::vector<int> prev(N, -1);

	std::vector<int> open;
	std::vector<bool> closed(N, false);

	int startIdx = Get1DIndex(start.x, start.y);
	int endIdx = Get1DIndex(end.x, end.y);

	// Optional safety: bounds
	if (startIdx < 0 || startIdx >= N || endIdx < 0 || endIdx >= N) return false;
	if ((int)teamGrid.size() != N) return false; // catches "not resized" bug

	// If end is not walkable *in team knowledge*, fail.
	// (Fog/Wall/etc should map to INT_MAX in GetTileCost)
	//if (GetTileCost(teamGrid[endIdx]) == INT_MAX)
	//	return false;

	gCost[startIdx] = 0;
	fCost[startIdx] = HeuristicManhattan(start, end);
	open.push_back(startIdx);

	int bestIdx = startIdx;
	int bestH = HeuristicManhattan(start, end);
	int bestG = 0;

	static int offsets[][2] = { {0,1}, {0,-1}, {-1,0}, {1,0} };

	while (!open.empty())
	{
		// find node in open with lowest fCost (tie-breaker: lower gCost)
		int bestPos = 0;
		for (int i = 1; i < (int)open.size(); ++i)
		{
			int a = open[i];
			int b = open[bestPos];

			if (fCost[a] < fCost[b] || (fCost[a] == fCost[b] && gCost[a] < gCost[b]))
				bestPos = i;
		}

		int currIdx = open[bestPos];
		open.erase(open.begin() + bestPos);

		if (closed[currIdx])
			continue;

		closed[currIdx] = true;

		{
			MazePt curr(currIdx % m_noGrid, currIdx / m_noGrid);
			int h = HeuristicManhattan(curr, end);
			int g = gCost[currIdx];

			if (h < bestH || (h == bestH && g < bestG))
			{
				bestH = h;
				bestG = g;
				bestIdx = currIdx;
			}
		}

		if (currIdx == endIdx)
		{  
			// reconstruct end -> start (INLINE #1)
			int walk = endIdx;
			while (walk != -1)
			{
				go->path.insert(go->path.begin(), MazePt(walk % m_noGrid, walk / m_noGrid));
				if (walk == startIdx) break;
				walk = prev[walk];
			}

			if (go->path.empty() ||
				!(go->path.front().x == start.x && go->path.front().y == start.y))
				return false;

			return true;
		}

		MazePt curr(currIdx % m_noGrid, currIdx / m_noGrid);

		for (int(&off)[2] : offsets)
		{
			MazePt next(curr.x + off[0], curr.y + off[1]);
			if (!IsWithinBoundary(next.x) || !IsWithinBoundary(next.y))
				continue;

			int nextIdx = Get1DIndex(next.x, next.y);

			if (closed[nextIdx])
				continue;

			int tileCost = GetTileCost(teamGrid[nextIdx]);   // <<< team memory here
			if (tileCost == INT_MAX)
				continue;

			int tentativeG = gCost[currIdx] + tileCost;

			if (tentativeG < gCost[nextIdx])
			{
				prev[nextIdx] = currIdx;
				gCost[nextIdx] = tentativeG;
				fCost[nextIdx] = tentativeG + HeuristicManhattan(next, end);

				if (std::find(open.begin(), open.end(), nextIdx) == open.end())
					open.push_back(nextIdx);
			}
		}
	}

	//end unreachable, go as far as i can
	{
		int walk = bestIdx;
		while (walk != -1)
		{
			go->path.insert(go->path.begin(), MazePt(walk % m_noGrid, walk / m_noGrid));
			if (walk == startIdx) break;
			walk = prev[walk];
		}

		// If bestIdx == startIdx (or reconstruction failed), path will be size 1 (or empty)
		if (go->path.empty() ||
			!(go->path.front().x == start.x && go->path.front().y == start.y))
		{
			go->path.clear();
		}
	}


	return false;
}

int SceneMovement_Week03::GetTileCost(Maze::TILE_CONTENT tile)
{

	if (!m_maze.IsPassable(tile)) return INT_MAX; 
	if (tile == Maze::TILE_EMPTY) return 1; // example
	return 1;
}

bool SceneMovement_Week03::TryFindFrontierTarget(GameObject* go, const MazePt& goal, MazePt& outTarget)
{
	bool found = false;
	int bestScore = INT_MAX;

	static int offsets[][2] = { {0,1}, {0,-1}, {-1,0}, {1,0} };

	// Pick team memory ONCE
	std::vector<Maze::TILE_CONTENT>& teamGrid =
		(go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;

	for (int y = 0; y < m_noGrid; ++y)
		for (int x = 0; x < m_noGrid; ++x)
		{
			if (x == go->curr.x && y == go->curr.y)
				continue;

			int idx = Get1DIndex(x, y);

			// must be known walkable in TEAM knowledge
			if (!m_maze.IsPassable(teamGrid[idx])) continue;

			// Frontier = known empty tile that touches fog in TEAM knowledge
			bool touchesFog = false;
			for (int(&off)[2] : offsets)
			{
				int nx = x + off[0], ny = y + off[1];
				if (!IsWithinBoundary(nx) || !IsWithinBoundary(ny)) continue;

				int nIdx = Get1DIndex(nx, ny);
				if (teamGrid[nIdx] == Maze::TILE_FOG)
				{
					touchesFog = true;
					break;
				}
			}
			if (!touchesFog) continue;

			// Pick frontier closest to final goal
			MazePt p(x, y);
			int score = HeuristicManhattan(p, goal);
			if (score < bestScore)
			{
				bestScore = score;
				outTarget = p;
				found = true;
			}
		}

	return found;
}

void SceneMovement_Week03::RevealAround(GameObject* go, int range)
{
	if (range < 0) return;

	MazePt curr = go->curr;
	go->visibleTargets.clear();
	// choose team grid ONCE
	std::vector<Maze::TILE_CONTENT>& teamGrid =
		(go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;

	// mark current as empty
	teamGrid[Get1DIndex(curr.x, curr.y)] = Maze::TILE_EMPTY;

	for (int dy = -range; dy <= range; ++dy)
	{
		for (int dx = -range; dx <= range; ++dx)
		{
			// Manhattan distance (grid steps)
			if (abs(dx) + abs(dy) > range) continue;

			MazePt p(curr.x + dx, curr.y + dy);
			if (!IsWithinBoundary(p.x) || !IsWithinBoundary(p.y)) continue;

			// (optional) skip the center tile if you want
			// if (dx == 0 && dy == 0) continue;

			if (auto t = RevealTileIfNew(go, p))
			{
				MessageNewTile msg(go, p, t);
				Handle(&msg);
			}

			GameObject* unit = RevealUnit(p);
			if (!unit) continue;
			if (unit == go) continue;
			if (unit->side == go->side) continue; // only enemies

			// Avoid duplicates (can happen if your reveal covers same tile twice in future changes)
			if (std::find(go->visibleTargets.begin(), go->visibleTargets.end(), unit) == go->visibleTargets.end())
				go->visibleTargets.push_back(unit);

			MessageRevealUnit msg(go, unit, p);
			Handle(&msg);
		}
	}
}

void SceneMovement_Week03::PathFind(GameObject* go, const MazePt& goal, int& moveBudget, int stopRange)
{
	if (moveBudget <= 0) return;

	auto& teamGrid = (go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;

	// Track where we are along the current plan
	size_t pathIndex = 1;   // next node to step to
	bool havePlan = false;
	bool planningToFrontier = false;

	while (moveBudget > 0)
	{
		// Stop if close enough
		if (HeuristicManhattan(go->curr, goal) <= stopRange)
			return;

		// ---- Decide whether we need to replan ----
		bool needReplan = false;

		if (!havePlan) needReplan = true;
		else if (go->path.size() <= 1) needReplan = true;
		else if (pathIndex >= go->path.size()) needReplan = true;                 // ran out of path
		else if (go->path[0].x != go->curr.x || go->path[0].y != go->curr.y) needReplan = true;   // plan no longer starts at our current tile
		else
		{
			// next step must be known & traversable (under your "known tiles only" rule)
			MazePt next = go->path[pathIndex];
			int tile = teamGrid[Get1DIndex(next.x, next.y)];
			if (tile == Maze::TILE_FOG) needReplan = true; // about to step into unknown
			else
			{
				int stepCost = GetTileCost(teamGrid[Get1DIndex(next.x, next.y)]);
				if (stepCost == INT_MAX) needReplan = true;  // treat as blocked/invalid if you use 0/negative for walls
			}
		}

		// ---- Replan only when needed ----
		if (needReplan)
		{

			havePlan = false;
			planningToFrontier = false;

			// ALWAYS try to go toward goal first.
			// AStar now returns a partial path even if goal is fog.
			AStar(go, go->curr, goal);

			// if we have at least 2 nodes, we can move (path[0]=curr, path[1]=next)
			havePlan = (go->path.size() > 1);

			if (!havePlan)
			{
				MazePt frontier;
				if (!TryFindFrontierTarget(go, goal, frontier))
					return;

				bool ok = AStar(go, go->curr, frontier);
				havePlan = (go->path.size() > 1);
				if (!havePlan) return;

				planningToFrontier = true;
			}

			pathIndex = 1;
		}

		// ---- Take exactly one step along current plan ----
		MazePt next = go->path[pathIndex];
		int stepCost = GetTileCost(teamGrid[Get1DIndex(next.x, next.y)]);

		if (stepCost > moveBudget)
			return;

		moveBudget -= stepCost;
		go->curr = next;
		++pathIndex;

		if (planningToFrontier)
		{
			RevealAround(go,go->viewRange);
		}
	}
}

void SceneMovement_Week03::DFSOnce(GameObject* go, int& moveBudget)
{
	MazePt curr{ go->curr.x, go->curr.y };
	std::vector<MazePt>& stack = go->stack; //get a short-hand to stack instead of constant use of indirection later
	if (stack.empty() ||
		stack.back().x != curr.x || stack.back().y != curr.y)
		//make sure we don't push node into stack when backtracking
		go->stack.push_back(curr);

	std::vector<bool>& teamVisited =
		(go->side == GameObject::SIDE_BLUE) ? b_visited : r_visited;
	int currIdx = Get1DIndex(curr.x, curr.y);
	teamVisited[currIdx] = true;
	//mark current node as visited
	
	//offsets: UP, DOWN, LEFT, RIGHT
	static int offsets[][2] = { { 0, 1 }, { 0, -1 }, { -1, 0 },
	{ 1, 0 } };
	//check each direction
	//int (& offset)[2] ---> offset is a reference to int[2]
	MazePt next{};
	for (int(&offset)[2] : offsets)
	{
		next.Set(curr.x + offset[0], curr.y + offset[1]);
		//only consider next node if it's unvisited
		if (IsWithinBoundary(next.x) && IsWithinBoundary(next.y))
		{
			//update agent's mental record of the maze
			int idx = Get1DIndex(next.x, next.y);

			if (teamVisited[idx])
				continue;


			std::vector<Maze::TILE_CONTENT>& teamGrid =
				(go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;

			teamGrid[idx] = m_maze.See(next);

			int stepCost = GetTileCost(teamGrid[idx]);
			if (stepCost == INT_MAX)
				continue; // blocked / not walkable

			if (stepCost > moveBudget)
				continue; // can't afford this step this turn

			// Move + spend cost
			moveBudget -= stepCost;

			if (m_maze.IsPassable(teamGrid[idx]))
			{
				go->curr = next; //let's move to the next cell
				return;
			}
		}
	}
	//already fully explored all surrounding neighbours. time tobacktrack
	stack.pop_back();
	if (!stack.empty())
		go->curr = stack.back();
}

Maze::TILE_CONTENT SceneMovement_Week03::RevealTileIfNew(GameObject* go, const MazePt& p)
{
	auto& teamGrid = (go->side == GameObject::SIDE_BLUE) ? b_grid : r_grid;
	int idx = Get1DIndex(p.x, p.y);

	if (teamGrid[idx] != Maze::TILE_FOG) return Maze::TILE_NULL;

	Maze::TILE_CONTENT t = m_maze.See(p);
	teamGrid[idx] = t;

	if (t == Maze::TILE_FOG) return Maze::TILE_NULL; // just in case
	return t;
}

GameObject* SceneMovement_Week03::RevealUnit(const MazePt& p)
{
	for (GameObject* other : m_goList)
	{
		if (!other || !other->active) continue;

		if (other->curr.x == p.x && other->curr.y == p.y)
			return other;
	}
	return nullptr;
}

bool SceneMovement_Week03::IsInAtkRange(GameObject* go, GameObject* target)
{
	if (go == NULL) return false;
	if (target == NULL) return false;

	MazePt a = go->curr;
	MazePt b = target->curr;   // if atkTarget is a GameObject*

	return HeuristicManhattan(a, b) <= go->atkRange;
}

GameObject* SceneMovement_Week03::PickClosestVisibleTarget(GameObject* go)
{
	if (!go) return nullptr;

	GameObject* best = nullptr;
	int bestD = INT_MAX;

	for (GameObject* t : go->visibleTargets)
	{
		if (!t || !t->active) continue;
		int d = HeuristicManhattan(go->curr, t->curr);
		if (d < bestD)
		{
			bestD = d;
			best = t;
		}
	}
	return best;
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
	timeCounter = 0;
}


//---------------------------FOR SPAWNERS SETTING UP ONLY-----------------------------------
void SceneMovement_Week03::SetTileBoth(int x, int y, Maze::TILE_CONTENT tileType)
{
	if (x < 0 || x >= m_noGrid || y < 0 || y >= m_noGrid) return; // adjust names
	int idx = Get1DIndex(x, y);
	m_maze.m_grid[idx] = tileType;
	m_myGrid[idx] = tileType;
}

void SceneMovement_Week03::Carve2x2Both(int x, int y, Maze::TILE_CONTENT tileType)
{
	SetTileBoth(x, y, tileType);
	SetTileBoth(x + 1, y, tileType);
	SetTileBoth(x, y + 1, tileType);
	SetTileBoth(x + 1, y + 1, tileType);
}

void SceneMovement_Week03::FillCurrNodes2x2FromWorld(GameObject* go)
{
	// Convert world pos to grid cell (top-left of the 2x2 footprint)
	// IMPORTANT: choose floor vs round depending on how you place objects.
	int gx = (int)std::floor((go->pos.x - m_gridOffset) / m_gridSize);
	int gy = (int)std::floor((go->pos.y - m_gridOffset) / m_gridSize);

	// Clamp so x+1, y+1 stay inside the grid
	//std::max
	gx = (((0) > ((((gx) < (m_noGrid - 2)) ? (gx) : (m_noGrid - 2)))) ? (0) : ((((gx) < (m_noGrid - 2)) ? (gx) : (m_noGrid - 2))));
	gy = (((0) > ((((gy) < (m_noGrid - 2)) ? (gy) : (m_noGrid - 2)))) ? (0) : ((((gy) < (m_noGrid - 2)) ? (gy) : (m_noGrid - 2))));

	// Fill the 4 nodes (2x2)
	go->currNodes[0].Set(gx, gy);
	go->currNodes[1].Set(gx + 1, gy);
	go->currNodes[2].Set(gx, gy + 1);
	go->currNodes[3].Set(gx + 1, gy + 1);

	// (optional) also keep go->curr in sync if you use it
	go->curr.Set(gx, gy);
}
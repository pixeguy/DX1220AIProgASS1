#ifndef SCENE_MOVEMENT_WEEK03_H
#define SCENE_MOVEMENT_WEEK03_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include <queue>

class SceneMovement_Week03 : public SceneBase , ObjectBase
{
public:
	SceneMovement_Week03();
	~SceneMovement_Week03();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void SetTileBoth(int x, int y, Maze::TILE_CONTENT tileType);

	void Carve2x2Both(int x, int y, Maze::TILE_CONTENT tileType);

	void FillCurrNodes2x2FromWorld(GameObject* go);

	//init objs
	virtual GameObject* InitMainBase(GameObject::SIDE side, Vector3 pos);
	GameObject* InitGoldenOrb(Vector3 pos);
	virtual GameObject* InitSpawner(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* InitMortarSpawner(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* SpawnUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type = GameObject::GO_NONE);
	virtual GameObject* SpawnMetalUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type = GameObject::GO_NONE);
	void SpawnGrid3x3(GameObject::SIDE side,const Vector3& centerPos,GameObject::GAMEOBJECT_TYPE type);
	MazePt SpawnMortarArea(GameObject::SIDE side);

	void RenderGOBar(GameObject* go, float vertScale);
	void RenderGOBar(GameObject* go, float vertScale, Vector3 pos);
	Vector3 RandomPointInRing(const Vector3& center, float minRadius, float maxRadius);

	void RenderGO(GameObject* go);
	bool Handle(Message* message);
	int HandleCount(Message* message);
	void CountingGO();
	void RenderDebugGO(GameObject::SIDE side, float y);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	GameObject* FetchProj(GameObject::GAMEOBJECT_TYPE type);

	int GetHealPriority(GameObject* go);
	std::string GameState();

	GameObject* ref; //delete later

	std::string gameStateString;

	struct ArmyStats
	{
		int countPerType[(int)GameObject::GAMEOBJECT_TYPE::GO_TOTAL] = { 0 };
		int total = 0;
		GameObject::SIDE side;
	};
	void CalcNeededUnits();
	int MechanicNeedGet(GameObject* spawner);
	ArmyStats ComputeArmyStats(GameObject::SIDE mySide);
	bool DecideSpawn(GameObject* spawner);

	float timeCounter = 0;

	// material thresholds
	const int lowWoodThreshold = 4; 
	const int lowMetalThreshold = 4;
	const int highWoodThreshold = 50;
	const int highMetalThreshold = 35;
	// ratios that i want
	const float attackerRatio = 0.23f;
	const float rangedRatio = 0.17f;
	const float supportRatio = 0.21f;
	const float mechanicRatio = 0.14f;
	const float tankRatio = 0.13f;
	const float mortarRatio = 0.12f;
	//priority of each unit
	const int prioAttacker = 4;
	const int prioRanged = 2;
	const int prioSupport = 3;
	const int prioMech = 5;
	const int prioTank = 3;
	const int prioMortar = 1;
	//cost of each unit
	const float maxCost = 6;
	const int costAttacker = 2;
	const int costRanged = 2;
	const int costSupport = 3;
	const int costMech = 2;
	const int costTank = 4;
	const int costMortar = 5;
	const float costWeight = 1.0f; // tweak this later

	bool gamePlaying = false;
	bool goldenEvent = false;

protected:
	std::vector<GameObject*> m_projList;
	std::vector<GameObject*> m_spawners;
	std::vector<GameObject*> m_goList;
	float m_speed;
	GameObject* m_ghost;
	int m_objectCount;
	float m_gridOffset;
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL] = { 0 };
	int m_numBlueGO[GameObject::GO_TOTAL] = { 0 };
	int m_numRedGO[GameObject::GO_TOTAL] = { 0 };
	float zOffset;



	void CarveUntilNoFog();
	void DFS(MazePt curr);
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;
	int HeuristicManhattan(const MazePt& a, const MazePt& b);
	bool BFSMaterial(GameObject* go, Maze::TILE_CONTENT tile, MazePt& outMat);
	bool AStar(GameObject* go, MazePt start, MazePt end);
	// Add this method declaration to SceneTurn.h inside the SceneTurn class
	int GetTileCost(Maze::TILE_CONTENT tile);

	bool TryFindFrontierTarget(GameObject* go, const MazePt& goal, MazePt& outTarget);

	void RevealAround(GameObject* go, int range);

	void PathFind(GameObject* go, const MazePt& goal, int& moveBudget, int stopRange);

	void ClearSpawnArea(const std::initializer_list<std::pair<int, int>>& patches);
	void DFSOnce(GameObject* go, int& moveBudget);

	Maze::TILE_CONTENT RevealTileIfNew(GameObject* go, const MazePt& p);

	GameObject* RevealUnit(const MazePt& p);

	bool IsInAtkRange(GameObject* go, GameObject* target);

	bool IsInAtkRange(GameObject* go, MazePt& targetPt);

	GameObject* PickClosestVisibleTarget(GameObject* go);

	GameObject::SIDE GetSide(GameObject* go);
	GameObject::SIDE m_activeSide = GameObject::SIDE_BLUE;
	int m_turn = 0;
	Maze m_maze;
	MazePt m_start;
	MazePt m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::vector<bool> b_visited;
	std::vector<bool> r_visited;
	std::vector<Maze::TILE_CONTENT> b_grid;
	std::vector<Maze::TILE_CONTENT> r_grid;
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector<MazePt> m_previous; //to store previous tile
	std::vector<MazePt> m_shortestPath;  //to store shortest path
	unsigned m_mazeKey;
	float m_wallLoad;
};

#endif
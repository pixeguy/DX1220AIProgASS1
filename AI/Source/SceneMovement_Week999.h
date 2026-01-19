#ifndef SCENE_MOVEMENT_WEEK999_H
#define SCENE_MOVEMENT_WEEK999_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include <queue>

class SceneMovement_Week999 : public SceneBase
{
public:
	SceneMovement_Week999();
	~SceneMovement_Week999();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	//init objs
	virtual GameObject* InitMainBase(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* InitSpawner(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* SpawnUnit(GameObject::SIDE side, Vector3 pos);

	void RenderGOBar(GameObject* go, float vertScale);
	void RenderGO(GameObject *go);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	GameObject* FetchProj();
protected:

	std::vector<GameObject*> m_projList;
	std::vector<GameObject*> m_spawners;
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL];
	float zOffset;

	void CarveUntilNoFog();
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;
	bool AStar(GameObject* go, MazePt start, MazePt end);
	// Add this method declaration to SceneTurn.h inside the SceneTurn class
	int GetTileCost(Maze::TILE_CONTENT tile);

	bool TryFindFrontierTarget(GameObject* go, const MazePt& goal, MazePt& outTarget);

	void RevealAround(GameObject* go);

	void PathFind(GameObject* go, const MazePt& goal, int moveBudget);

	int m_turn = 0;

	Maze m_maze;
	MazePt m_start;
	MazePt m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector<MazePt> m_previous; //to store previous tile
	std::vector<MazePt> m_shortestPath;  //to store shortest path
	unsigned m_mazeKey;
	float m_wallLoad;
};

#endif
#ifndef SCENE_TURN_H
#define SCENE_TURN_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Maze.h"
#include <queue>

class SceneTurn : public SceneBase
{
public:
	SceneTurn();
	~SceneTurn();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	void CarveUntilNoFog();
	void DFS(MazePt curr);
	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE);

	bool BFS(MazePt start, MazePt end);

protected:
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;

	// Exercise Week 8
	// 5.	Declare these methods and variable in SceneTurn.h
	void DFSOnce(GameObject* go);
	bool BFSLimit(GameObject* go, MazePt end, int limit);
	bool AStar(GameObject* go, MazePt start, MazePt end);
    // Add this method declaration to SceneTurn.h inside the SceneTurn class
    int GetTileCost(Maze::TILE_CONTENT tile);

	bool TryFindFrontierTarget(GameObject* go, const MazePt& goal, MazePt& outTarget);

	void RevealAround(GameObject* go);

	void PathFind(GameObject* go, const MazePt& goal, int moveBudget);

	int m_turn = 0;

	std::vector<GameObject *> m_goList;
	float m_speed;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

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
	// Exercise Week 8
	// 5.	Declare these methods and variable in SceneTurn.h



};

#endif
#ifndef SCENE_MOVEMENT_WEEK999_H
#define SCENE_MOVEMENT_WEEK999_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

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

	void RenderGO(GameObject *go);
	void RenderGOBar(GameObject* go, float vertScale);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	GameObject* FetchProj();
protected:

	std::vector<GameObject*> m_projList;
	std::vector<GameObject *> m_goList;
	std::vector<GameObject *> m_spawners;
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
};

#endif
#ifndef SCENE_MOVEMENT_WEEK03_H
#define SCENE_MOVEMENT_WEEK03_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

class SceneMovement_Week03 : public SceneBase , ObjectBase
{
public:
	SceneMovement_Week03();
	~SceneMovement_Week03();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	//init objs
	virtual GameObject* InitMainBase(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* InitSpawner(GameObject::SIDE side, Vector3 pos);
	virtual GameObject* SpawnUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type = GameObject::GO_NONE);
	virtual GameObject* SpawnMetalUnit(GameObject::SIDE side, Vector3 pos, GameObject::GAMEOBJECT_TYPE type = GameObject::GO_NONE);


	void RenderGOBar(GameObject* go, float vertScale);
	Vector3 RandomPointInRing(const Vector3& center, float minRadius, float maxRadius);

	void RenderGO(GameObject* go);
	bool Handle(Message* message);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	GameObject* FetchProj();

protected:
	std::vector<GameObject*> m_projList;
	std::vector<GameObject*> m_spawners;
	std::vector<GameObject*> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject* m_ghost;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL];
	float zOffset;
};

#endif
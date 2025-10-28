#ifndef SCENE_MOVEMENT_WEEK03_H
#define SCENE_MOVEMENT_WEEK03_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneMovement_Week03 : public SceneBase
{
public:
	SceneMovement_Week03();
	~SceneMovement_Week03();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject* go);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
protected:

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
	int objectCount = 0;
	int fishCount = 0;
};

#endif
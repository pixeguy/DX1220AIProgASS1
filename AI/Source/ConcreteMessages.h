#ifndef CONCRETE_MESSAGE_H
#define CONCRETE_MESSAGE_H

#include "Message.h"
#include "GameObject.h"

struct MessageWRU : public Message
{
	enum SEARCH_TYPE
	{
		SEARCH_NONE = 0,
		NEAREST_SHARK,
		NEAREST_FISHFOOD,
		NEAREST_FULLFISH,
		HIGHEST_ENERGYFISH,
		NEAREST_SPAWNER,
		NEAREST_ENEMY,
		NEAREST_ALLY_NOSUP,
		FURTHEST_FRONTLINE,
		NEAREST_MORTAR_ENEMY,
		NEAREST_FREE_SUP,
		NEAREST_URG_SUP,
		NEAREST_ALLY_ATTACKER,
		ALLYACTIVECOUNT,
		NEAREST_MECHANIC_HEAL,
		NEAREST_ENEMY_INAREA,
		NEAREST_OBJ,
		NEAREST_MAT,
		ATTACKERCOUNT,
		NEAREST_GOLDENORB,
	};
	MessageWRU(GameObject *goValue, SEARCH_TYPE typeValue, float thresholdValue) : go(goValue), type(typeValue), threshold(thresholdValue) {}
	MessageWRU(GameObject* goValue, SEARCH_TYPE typeValue, float thresholdValue, float tooCloseValue) : go(goValue), type(typeValue), threshold(thresholdValue), mortarTooCloseValue(tooCloseValue) {}
	virtual ~MessageWRU() {}

	GameObject *go;
	SEARCH_TYPE type;
	float threshold;
	float mortarTooCloseValue;
};

struct MessageNewTile : public Message
{
	MessageNewTile(GameObject* goValue, const MazePt& posValue, Maze::TILE_CONTENT tileValue)
		: go(goValue), pos(posValue), tile(tileValue) {
	}

	virtual ~MessageNewTile() {}

	GameObject* go;
	MazePt pos;
	Maze::TILE_CONTENT tile;
};

struct MessageRevealUnit : public Message
{
	MessageRevealUnit(GameObject* spotterValue, GameObject* targetValue, const MazePt& posValue)
		: spotter(spotterValue), target(targetValue), pos(posValue) {
	}

	GameObject* spotter;
	GameObject* target;
	MazePt pos;
};

struct MessageCheckActive : public Message
{
	MessageCheckActive() {}
};

struct MessageCheckFish : public Message
{
	MessageCheckFish() {}
};

struct MessageCheckFood : public Message
{
	MessageCheckFood() {}
};

struct MessageCheckShark : public Message
{
	MessageCheckShark() {}
};

//week 5
//this message asks the scene to spawn an object
struct MessageSpawn : public Message
{
	// owner of msg, what to spawn, # to spawn, # tiles(x & y) from owner
	// passing range array by reference to avoid array decay (to int*) - that way we can force users to only pass an array of size 2(no other sizes will be accepted)
	// alternatively, look into std::array(c++11 onwards)?
	MessageSpawn(GameObject* goVal, int typeVal, int countVal, int (&range)[2]) : go(goVal), type(typeVal), count(countVal)
	{
		distRange[0] = range[0];
		distRange[1] = range[1];
	}

	int distRange[2];
	int type;
	int count;
	GameObject* go;
};

struct MessageSpawnFood : public Message
{
	// owner of msg, what to spawn, # to spawn, # tiles(x & y) from owner
	// passing range array by reference to avoid array decay (to int*) - that way we can force users to only pass an array of size 2(no other sizes will be accepted)
	// alternatively, look into std::array(c++11 onwards)?
	MessageSpawnFood(GameObject* goVal, int typeVal, int countVal, int(&range)[2]) : go(goVal), type(typeVal), count(countVal)
	{
		distRange[0] = range[0];
		distRange[1] = range[1];
	}

	int distRange[2];
	int type;
	int count;
	GameObject* go;
};

struct MessageStop : public Message
{
	MessageStop(GameObject* specFishFood) {
		go = specFishFood;
	}

	GameObject* go;
};

//this message is meant to turn food into fish
struct MessageEvolve : public Message
{
	MessageEvolve(GameObject* goVal) : go(goVal) {}

	GameObject* go;
};

struct MessageAskHelp : public Message
{
	MessageAskHelp(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnProj : public Message
{
	MessageSpawnProj(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnMorBomb : public Message
{
	MessageSpawnMorBomb(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnBigMorBomb : public Message
{
	MessageSpawnBigMorBomb(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnProjTank : public Message
{
	MessageSpawnProjTank(GameObject* goVal, GameObject* targetgo, bool rocketOrProj) : go(goVal), target(targetgo), rocketOrProj(rocketOrProj) {}
	GameObject* go;
	GameObject* target;
	bool rocketOrProj;
};

struct MessageAskForAtk : public Message
{
	MessageAskForAtk(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnAttacker : public Message
{
	MessageSpawnAttacker(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageSpawnMortar : public Message
{
	MessageSpawnMortar(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageHowManyUnit : public Message
{
	MessageHowManyUnit(GameObject::GAMEOBJECT_TYPE type ,GameObject::SIDE side) : go_type(type), side(side){}
	GameObject::GAMEOBJECT_TYPE go_type;
	GameObject::SIDE side;
};

struct MessageMechanicBuild : public Message
{
	MessageMechanicBuild(GameObject* goVal) : go(goVal) {}
	GameObject* go;
};

struct MessageMeteorSpawn : public Message
{
	MessageMeteorSpawn(MazePt* coord) : coord(coord) {}
	MazePt* coord;
};

#endif

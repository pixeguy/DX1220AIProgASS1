#include "GameObject.h"
#include "ConcreteMessages.h"
#include "GridSettings.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue) 
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	moveSpeed(1.f),
	energy(10.f),
	sm(NULL),
	nearest(NULL),
	nextState(nullptr),
	currentState(nullptr),
	currNode(0)
{
	static int count = 0;
	id = ++count;
	moveLeft = moveRight = moveUp = moveDown = true;
}

GameObject::~GameObject()
{
}

bool GameObject::EnergyReduce(float cost)
{
	energy += cost;
	if (energy >= maxEnergy)
	{
		energy = 0.f;
		return true; // ready to act
	}
	return false;
}

//week 4
bool GameObject::Handle(Message* message)
{
	//let's check if message is MessageCheckActive
	if (dynamic_cast<MessageCheckActive*>(message) != nullptr)
		return active;
	else if (dynamic_cast<MessageCheckFish*>(message) != nullptr)
		return active && type == GameObject::GO_FISH;
	else if (dynamic_cast<MessageCheckFood*>(message) != nullptr)
		return active && type == GameObject::GO_FISHFOOD;
	else if (dynamic_cast<MessageCheckShark*>(message) != nullptr)
		return active && type == GameObject::GO_SHARK;
	//week 5
	//set speed to 0 upon receiving stop message
	else if (dynamic_cast<MessageStop*>(message) != nullptr)
	{
		moveSpeed = 0;
		return true;
	}
	else if (dynamic_cast<MessageAskHelp*>(message) != nullptr)
	{
		MessageAskHelp* msg = static_cast<MessageAskHelp*>(message);
		healTarget = msg->go;
		nearest = msg->go;
		sm->SetNextState("Healing");
		if (msg->go->urgent) {
			urgent = true; //set supporters to also urgent, both urgent
			sm->SetNextState("UrgentHealing");
		}
		sm->m_currState->Exit();
		sm->m_currState = sm->m_nextState;
		sm->m_currState->Enter();
		return true;
	}
	else if (dynamic_cast<MessageAskForAtk*>(message) != nullptr)
	{
		MessageAskForAtk* msg = static_cast<MessageAskForAtk*>(message);
		if(msg->go->lastAttacker == NULL)
			return false; //no attackers to help attack
		nearest = msg->go->lastAttacker;
		urgent = true; //attackers always urgent when called for help
		return true;
	}

	//note: pardon the inconsistency(when compared to SceneMovement's Handle)
	//we do NOT want to create a new message on the heap PER object for performance reasons
	return false;
}

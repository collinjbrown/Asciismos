// This might seem a little strange, so we'll have to explain why this is set up
// the way it is...

// Essentially, we have a few types of objects here: entities, components, systems, component blocks, and the ECS hub.
// Entities are (ideally) just ids, though in our implementation they have a little bit more info packed into them. They're not referenced directly all that often (except for the player).
// Components are the various parts that constitute the entity; they can be added and removed on the fly, though they need to be registered with the ECS hub when they are.
// Systems hold two things: a vector with all the components (that fall within their domain) and the update function that loops over these components and actually *does* whatever the system is meant to do.
// For example, the physics system loops over all the physics components and applies gravity and friction and whatnot to them.
// Component blocks are admittedly a little janky, and perhaps I'll find some way to remove them, but they're a sort of interface between the ECS hub and individual systems.
// Each component block contains a system and the ID of the component-type it should hold. This allows the ECS hub to assign new components to the correct system.
// The ECS hub holds component blocks; it is also where we instantiate various things, though that happens in ecs.cpp.
// In short, when the game starts, the ECS hub runs its Init() function, where we create systems and component blocks and assign the former to the latter.
// Then, Main calls the ECS hub's update function which in turn calls the update function on each component block which in turn calls the update function on each system.
// There, the system loops through each component and applies some logic to it.
// To add a new component, one calls the RegisterComponent() function and passes in the component and its entity.
// Then, the ECS hub takes this and determines---via the component ID---which component block it belongs to.
// It calls the AddComponent() function in the block which then calls the AddComponent() function in the system
// which finally converts the abstract component into its respective type and adds it to its component list (and then iterates over it during its update).

// In short, if one adds a new component, one needs to assign it a new component ID in component.h, then add it to the forward declarations in system.h.
// Then, if necessary, one can create a system to manage that component. This involves adding it to system.h, then defining it in the last section of ecs.cpp,
// then one needs to go to the ECS section of ecs.cpp and instantiate the system (and its respective component block) in the Init() function.
// This might sound complicated, but it really honestly isn't (though I will say this is probably more complicated than it needs to be).

#ifndef ECS_H
#define ECS_H

#include <vector>
#include <map>

using namespace std;

class Entity;
class System;
class Component;

#pragma region Nodes

struct Node
{
	int x;
	int y;
	int parentX;
	int parentY;

	bool blocked;
	bool ground;

	float g;
	float h;
	float F() { return g + h; }

	Node(int x, int y)
	{
		this->x = x;
		this->y = y;

		this->parentX = 0;
		this->parentY = 0;

		blocked = false;
		ground = false;

		this->g = INFINITY;
		this->h = 0.0f;
	}
};

inline bool operator < (const Node& lhs, const Node& rhs)
{
	return (lhs.g + lhs.h) < (rhs.g + rhs.h);
}

#pragma endregion

class ComponentBlock
{
public:
	System* system;
	int componentID;

	void Update(int activeScene, float deltaTime);
	void AddComponent(Component* c);
	void PurgeEntity(Entity* e);
	ComponentBlock(System* system, int componentID);
};

class ECS
{
private:
	uint32_t entityIDCounter = 0;
	int round = 0;
	static const int mWidth = 100;
	static const int mHeight = 100;

public:
	static ECS main;
	int activeScene;
	Entity* player;

	vector<Entity*> entities;
	vector<Entity*> dyingEntities;

	float nodeSize = 5.0f;
	Node* nodeMap[mWidth][mHeight];

	vector<ComponentBlock*> componentBlocks;

	uint32_t GetID();
	void Init();
	void Update(float deltaTime);
	Entity* CreateEntity(int scene, std::string name);
	void DeleteEntity(Entity* e);
	void AddDeadEntity(Entity* e);
	void PurgeDeadEntities();
	void RegisterComponent(Component* component, Entity* entity);
};

#endif

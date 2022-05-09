#ifndef SYSTEM_H
#define SYSTEM_H

// As I mentioned in ecs.h, this contains some forward declarations of the components
// to satisfy the compiler. Otherwise, this just contains a whole lot of declarations
// we'll define in ecs.cpp, which really handles all the system logic.
// We'll also throw any structs and classes needed to handle particular system logic in here,
// like the collision struct used by the collision system.

#include "game.h"
#include <vector>
#include <array>
#include "glm/gtx/norm.hpp"

using namespace std;

class Component;
class PositionComponent;
class PhysicsComponent;
class StaticSpriteComponent;
class InputComponent;
class CameraFollowComponent;
class AnimationComponent;
class AnimationControllerComponent;
class PlayerAnimationControllerComponent;
class ParticleComponent;
class AIComponent;
class ImageComponent;
class Entity;

class System
{
public:
	virtual void Update(int activeScene, float deltaTime) = 0;
	virtual void AddComponent(Component* component) = 0;
	virtual void PurgeEntity(Entity* e) = 0;
};

class StaticRenderingSystem : public System
{
public:
	vector<StaticSpriteComponent*> sprites;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class InputSystem : public System
{
public:
	vector<InputComponent*> move;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class CameraFollowSystem : public System
{
public:
	vector<CameraFollowComponent*> folls;

	void Update(int activeScene, float deltaTime);

	float Lerp(float a, float b, float t);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class AnimationControllerSystem : public System
{
public:
	vector<AnimationControllerComponent*> controllers;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class AnimationSystem : public System
{
public:
	vector<AnimationComponent*> anims;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class ParticleSystem : public System
{
public:
	vector<ParticleComponent*> particles;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class AISystem : public System
{
public:
	vector<AIComponent*> ai;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class ImageSystem : public System
{
public:
	vector<ImageComponent*> images;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};
#endif

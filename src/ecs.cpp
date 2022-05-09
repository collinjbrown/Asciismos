// ecs.cpp is the meat-and-potatoes of the game. In order, it contains:
// - Several utility functions that are used by a number of systems and components.
// - The definitions of some entity functions.
// - The definitions of some component-block functions.
// - The init and update functions (among others) which create systems and assign components to them respectively.
// -- The latter of these is where we instantiate all objects that should exist from the first frame to the last (or at least for quite a while).
// - The definitions of some component functions.
// -- Some of these components also contain functions that need to be accessed via the component itself, rather than through its respective system.
// - The definitions of systems and their functions.
// -- This last section, I'd reckon, is the largest of them and is where most of the game's logic resides.

#include "particleengine.h"
#include "ecs.h"
#include "system.h"
#include "component.h"
#include "entity.h"
#include <algorithm>

#pragma region Utility

float Norm(glm::vec2 a)
{
	return sqrt(a.x * a.x + a.y * a.y);
}

glm::vec2 Normalize(glm::vec2 a)
{
	return a * (1 / Norm(a));
}

glm::vec2 lerp(glm::vec2 pos, glm::vec2 tar, float step)
{
	return (pos * (1.0f - step) + (tar * step));
}

float Dot(glm::vec2 a, glm::vec2 b)
{
	return a.x * b.x + a.y * b.y;
}
#pragma endregion

#pragma region Entities

int Entity::Get_ID() { return ID; }
int Entity::Get_Scene() { return scene; }
std::string Entity::Get_Name() { return name; }

void Entity::Set_ID(int newID) { ID = newID; }
void Entity::Set_Scene(int newScene) { scene = newScene; }
void Entity::Set_Name(std::string newName) { name = newName; }

Entity:: Entity(int ID, int scene, std::string name)
{
	this->ID = ID;
	this->scene = scene;
	this->name = name;
};

#pragma endregion

#pragma region Component Blocks
void ComponentBlock::Update(int activeScene, float deltaTime)
{
	system->Update(activeScene, deltaTime);
}
void ComponentBlock::AddComponent(Component* c)
{
	system->AddComponent(c);
}
void ComponentBlock::PurgeEntity(Entity* e)
{
	system->PurgeEntity(e);
}
ComponentBlock::ComponentBlock(System* system, int componentID)
{
	this->system = system;
	this->componentID = componentID;
}
#pragma endregion

#pragma region ECS
uint32_t ECS::GetID()
{
	return ++entityIDCounter;
}

void ECS::Init()
{
	// I think we're going to have to initiate every component block
	// at the beginning of the game. This might be long.

	InputSystem* inputSystem = new InputSystem();
	ComponentBlock* inputBlock = new ComponentBlock(inputSystem, inputComponentID);
	componentBlocks.push_back(inputBlock);

	ParticleSystem* particleSystem = new ParticleSystem();
	ComponentBlock* particleBlock = new ComponentBlock(particleSystem, particleComponentID);
	componentBlocks.push_back(particleBlock);

	ImageSystem* imageSystem = new ImageSystem();
	ComponentBlock* imageBlock = new ComponentBlock(imageSystem, imageComponentID);
	componentBlocks.push_back(imageBlock);

	StaticRenderingSystem* renderingSystem = new StaticRenderingSystem();
	ComponentBlock* renderingBlock = new ComponentBlock(renderingSystem, spriteComponentID);
	componentBlocks.push_back(renderingBlock);

	CameraFollowSystem* camfollowSystem = new CameraFollowSystem();
	ComponentBlock* camfollowBlock = new ComponentBlock(camfollowSystem, cameraFollowComponentID);
	componentBlocks.push_back(camfollowBlock);

	AnimationControllerSystem* animationControllerSystem = new AnimationControllerSystem();
	ComponentBlock* animationControllerBlock = new ComponentBlock(animationControllerSystem, animationControllerComponentID);
	componentBlocks.push_back(animationControllerBlock);

	AnimationSystem* animationSystem = new AnimationSystem();
	ComponentBlock* animationBlock = new ComponentBlock(animationSystem, animationComponentID);
	componentBlocks.push_back(animationBlock);
}

void ECS::Update(float deltaTime)
{
	round++;

	if (round == 1)
	{
		#pragma region UI Instantiation

		Entity* alphaWatermark = CreateEntity(0, "Watermark");
		Texture2D* watermark = Game::main.textureMap["watermark"];
		Texture2D* watermarkMap = Game::main.textureMap["watermarkMap"];

		ECS::main.RegisterComponent(new PositionComponent(alphaWatermark, true, true, 0, 0, 100, 0), alphaWatermark);
		ECS::main.RegisterComponent(new StaticSpriteComponent(alphaWatermark, true, (PositionComponent*)alphaWatermark->componentIDMap[positionComponentID], watermark->width, watermark->height, 1.0f, 1.0f, watermark, watermarkMap, false, false, false), alphaWatermark);
		ECS::main.RegisterComponent(new ImageComponent(alphaWatermark, true, Anchor::topRight, 0, 0), alphaWatermark);


		#pragma endregion
	}

	for (int i = 0; i < componentBlocks.size(); i++)
	{
		componentBlocks[i]->Update(activeScene, deltaTime);
	}

	PurgeDeadEntities();
}

// We probably aren't actually gonna use this, but I'll leave it here just in case.
//void ECS::CreateNodeMap()
//{
//	for (int x = 0; x < mWidth; x++)
//	{
//		for (int y = 0; y < mHeight; y++)
//		{
//			bool blocked = false;
//			Node* n = new Node(x, y);
//
//			for (int e = 0; e < entities.size(); e++)
//			{
//				PositionComponent* pos = (PositionComponent*)entities[e]->componentIDMap[positionComponentID];
//				ColliderComponent* col = (ColliderComponent*)entities[e]->componentIDMap[colliderComponentID];
//
//				blocked = (PointOverlapRect(glm::vec2(x * nodeSize, y * nodeSize), glm::vec2(pos->x, pos->y) + glm::vec2(col->offsetX, col->offsetY), col->width, col->height) && !col->trigger && pos->stat);
//				
//				if (blocked)
//				{
//					n->col = col;
//				}
//
//				nodeMap[x][y] = n;
//			}
//		}
//	}
//}

void ECS::AddDeadEntity(Entity* e)
{
	if (std::find(dyingEntities.begin(), dyingEntities.end(), e) == dyingEntities.end())
	{
		dyingEntities.push_back(e);
	}
}

void ECS::PurgeDeadEntities()
{
	if (dyingEntities.size() > 0)
	{
		int n = dyingEntities.size();

		for (int i = 0; i < n; i++)
		{
			DeleteEntity(dyingEntities[i]);
		}

		dyingEntities.clear();
	}
}

Entity* ECS::CreateEntity(int scene, std::string name)
{
	Entity* e = new Entity(GetID(), scene, name);
	return e;
}

void ECS::DeleteEntity(Entity* e)
{
	for (int i = 0; i < componentBlocks.size(); i++)
	{
		componentBlocks[i]->PurgeEntity(e);
	}

	delete e;
}

void ECS::RegisterComponent(Component* component, Entity* entity)
{
	entity->components.push_back(component);
	entity->componentIDMap.emplace(component->ID, component);

	for (int i = 0; i < componentBlocks.size(); i++)
	{
		if (componentBlocks[i]->componentID == component->ID)
		{
			componentBlocks[i]->AddComponent(component);
			return;
		}
	}
}
#pragma endregion

#pragma region Components

#pragma region Position Component
glm::vec2 PositionComponent::Rotate(glm::vec2 point)
{
	glm::vec3 forward = glm::vec3();
	glm::vec3 up = glm::vec3();
	glm::vec3 right = glm::vec3();

	if (rotation != 0)
	{
		float radians = rotation * (M_PI / 180.0f);

		forward = glm::vec3(0, 0, 1);
		right = glm::vec3(cos(radians), sin(radians), 0);
		up = glm::cross(forward, right);
	}
	else
	{
		up = glm::vec3(0, 1, 0);
		right = glm::vec3(1, 0, 0);
	}

	return RelativeLocation(point, up, right);
}

glm::vec2 PositionComponent::RelativeLocation(glm::vec2 p, glm::vec2 up, glm::vec2 right)
{
	return glm::vec2((p.x * right.x) + (p.y * up.x), (p.x * right.y) + (p.y * up.y));
}

PositionComponent::PositionComponent(Entity* entity, bool active, bool stat, float x, float y, float z, float rotation)
{
	ID = positionComponentID;
	this->active = active;
	this->entity = entity;
	this->stat = stat;
	this->x = x;
	this->y = y;
	this->z = z;
	this->rotation = rotation;
}
#pragma endregion

#pragma region Static Sprite Component

StaticSpriteComponent::StaticSpriteComponent(Entity* entity, bool active, PositionComponent* pos, float width, float height, float scaleX, float scaleY, Texture2D* sprite, Texture2D* mapTex, bool flippedX, bool flippedY, bool tiled)
{
	ID = spriteComponentID;
	this->active = active;
	this->entity = entity;
	this->pos = pos;

	this->width = width;
	this->height = height;
	
	this->scaleX = scaleX;
	this->scaleY = scaleY;

	this->sprite = sprite;
	this->mapTex = mapTex;

	this->flippedX = flippedX;
	this->flippedY = flippedY;

	this->tiled = tiled;
}

#pragma endregion

#pragma region Input Component

InputComponent::InputComponent(Entity* entity, bool active, bool acceptInput)
{
	this->ID = inputComponentID;
	this->active = active;
	this->entity = entity;

	this->acceptInput = acceptInput;
}

#pragma endregion

#pragma region Camera Follow Component

CameraFollowComponent::CameraFollowComponent(Entity* entity, bool active, float speed)
{
	this->ID = cameraFollowComponentID;
	this->active = active;
	this->entity = entity;

	this->speed = speed;
}

#pragma endregion

#pragma region Animation Component

void AnimationComponent::SetAnimation(std::string s)
{
	if (animations[s] != NULL)
	{
		activeAnimation = s;
		activeX = 0;
		activeY = animations[s]->rows - 1;
		lastTick = 0;
	}
}

void AnimationComponent::AddAnimation(std::string s, Animation2D* anim)
{
	animations.emplace(s, anim);
}

AnimationComponent::AnimationComponent(Entity* entity, bool active, PositionComponent* pos, Animation2D* idleAnimation, std::string animationName, Texture2D* mapTex, float scaleX, float scaleY, bool flippedX, bool flippedY)
{
	this->ID = animationComponentID;
	this->entity = entity;
	this->active = active;

	this->lastTick = 0;
	this->activeX = 0;
	this->activeY = 0;

	this->pos = pos;

	this->scaleX = scaleX;
	this->scaleY = scaleY;

	this->flippedX = flippedX;
	this->flippedY = flippedY;

	this->activeAnimation = animationName;
	this->animations.emplace(animationName, idleAnimation);
	this->activeY = animations[activeAnimation]->rows - 1;

	this->mapTex = mapTex;
}

#pragma endregion

#pragma region Player Animation Controller Component

PlayerAnimationControllerComponent::PlayerAnimationControllerComponent(Entity* entity, bool active, AnimationComponent* animator)
{
	this->ID = animationControllerComponentID;
	this->subID = exampleAnimControllerSubID;
	this->entity = entity;
	this->active = active;

	this->animator = animator;
}

#pragma endregion

#pragma region Particle Component

ParticleComponent::ParticleComponent(Entity* entity, bool active, float tickRate, float xOffset, float yOffset, int number, Element element, float minLifetime, float maxLifetime)
{
	this->ID = particleComponentID;
	this->entity = entity;
	this->active = active;

	this->lastTick = 0.0f;
	this->tickRate = tickRate;

	this->xOffset = xOffset;
	this->yOffset = yOffset;

	this->number = number;

	this->element = element;
	
	this->minLifetime = minLifetime;
	this->maxLifetime = maxLifetime;
}

#pragma endregion

#pragma region Image Component

ImageComponent::ImageComponent(Entity* entity, bool active, Anchor anchor, float x, float y)
{
	this->ID = imageComponentID;
	this->entity = entity;
	this->active = active;
	
	this->anchor = anchor;
	this->x = x;
	this->y = y;
}

#pragma endregion

#pragma endregion

#pragma region Systems

#pragma region Static Rendering System

void StaticRenderingSystem::Update(int activeScene, float deltaTime)
{
	std::sort(sprites.begin(), sprites.end(), [](StaticSpriteComponent* a, StaticSpriteComponent* b)
		{
			return a->pos->z < b->pos->z;
		});

	for (int i = 0; i < sprites.size(); i++)
	{
		StaticSpriteComponent* s = sprites[i];

		if (s->active && s->entity->Get_Scene() == activeScene ||
			s->active && s->entity->Get_Scene() == 0)
		{
			PositionComponent* pos = s->pos;

			if (pos->x + (s->width / 2.0f) > Game::main.leftX && pos->x - (s->width / 2.0f) < Game::main.rightX &&
				pos->y + (s->height / 2.0f) > Game::main.bottomY && pos->y - (s->height / 2.0f) < Game::main.topY &&
				pos->z < Game::main.camZ)
			{
				Game::main.renderer->prepareQuad(pos, s->width, s->height, s->scaleX, s->scaleY, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), s->sprite->ID, s->mapTex->ID, s->tiled, s->flippedX, s->flippedY);
			}
		}
	}
}

void StaticRenderingSystem::AddComponent(Component* component)
{
	sprites.push_back((StaticSpriteComponent*)component);
}

void StaticRenderingSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < sprites.size(); i++)
	{
		if (sprites[i]->entity == e)
		{
			StaticSpriteComponent* s = sprites[i];
			sprites.erase(std::remove(sprites.begin(), sprites.end(), s), sprites.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Input System

void InputSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < move.size(); i++)
	{
		InputComponent* m = move[i];

		if (m->active && m->entity->Get_Scene() == activeScene ||
			m->active && m->entity->Get_Scene() == 0)
		{
			
		}
	}
}

void InputSystem::AddComponent(Component* component)
{
	move.push_back((InputComponent*)component);
}

void InputSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < move.size(); i++)
	{
		if (move[i]->entity == e)
		{
			InputComponent* s = move[i];
			move.erase(std::remove(move.begin(), move.end(), s), move.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Camera Follow System

void CameraFollowSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < folls.size(); i++)
	{
		CameraFollowComponent* f = folls[i];

		if (f->active && f->entity->Get_Scene() == activeScene ||
			f->active && f->entity->Get_Scene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)f->entity->componentIDMap[positionComponentID];

			Game::main.camX = Lerp(Game::main.camX, pos->x, f->speed * deltaTime);
			Game::main.camY = Lerp(Game::main.camY, pos->y, f->speed * deltaTime);
		}
	}
}

float CameraFollowSystem::Lerp(float a, float b, float t)
{
	return (1 - t) * a + t * b;
}

void CameraFollowSystem::AddComponent(Component* component)
{
	folls.push_back((CameraFollowComponent*)component);
}

void CameraFollowSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < folls.size(); i++)
	{
		if (folls[i]->entity == e)
		{
			CameraFollowComponent* s = folls[i];
			folls.erase(std::remove(folls.begin(), folls.end(), s), folls.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Animation Controller System

void AnimationControllerSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < controllers.size(); i++)
	{
		AnimationControllerComponent* c = controllers[i];

		if (c->active && c->entity->Get_Scene() == activeScene ||
			c->active && c->entity->Get_Scene() == 0)
		{

			if (c->subID == exampleAnimControllerSubID)
			{
				/*if (abs(p->velocityX) < 100.0f && !move->crouching && col->onPlatform && move->canMove && c->animator->activeAnimation != s + "idle")
				{
					c->animator->SetAnimation(s + "idle");
				}*/
			}
		}
	}
}

void AnimationControllerSystem::AddComponent(Component* component)
{
	controllers.push_back((AnimationControllerComponent*)component);
}

void AnimationControllerSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < controllers.size(); i++)
	{
		if (controllers[i]->entity == e)
		{
			AnimationControllerComponent* s = controllers[i];
			controllers.erase(std::remove(controllers.begin(), controllers.end(), s), controllers.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Animation System

void AnimationSystem::Update(int activeScene, float deltaTime)
{
	std::sort(anims.begin(), anims.end(), [](AnimationComponent* a, AnimationComponent* b)
		{
			return a->pos->z < b->pos->z;
		});

	for (int i = 0; i < anims.size(); i++)
	{
		// Animations work by taking a big-ass spritesheet
		// and moving through the uvs by increments equal
		// to one divided by the width and height of each sprite;
		// this means we need to know how many such cells are in
		// the whole sheet (for both rows and columns), so that
		// we can feed the right cell coordinates into the
		// renderer. This shouldn't be too difficult; the real
		// question is how we'll manage conditions for different
		// animations.
		// We could just have a map containing strings and animations
		// and set the active animation by calling some function, sending
		// to that the name of the requested animation in the form of that
		// string, but that doesn't seem like the ideal way to do it.
		// We might try that first and then decide later whether
		// there isn't a better way to handle this.

		AnimationComponent* a = anims[i];

		if (a->active && a->entity->Get_Scene() == activeScene ||
			a->active && a->entity->Get_Scene() == 0)
		{
			a->lastTick += deltaTime;

			Animation2D* activeAnimation = a->animations[a->activeAnimation];

			int cellX = a->activeX, cellY = a->activeY;

			if (activeAnimation->speed < a->lastTick)
			{
				a->lastTick = 0;

				if (a->activeX + 1 < activeAnimation->rowsToCols[cellY])
				{
					cellX = a->activeX += 1;
				}
				else
				{
					if (activeAnimation->loop ||
						a->activeY > 0)
					{
						cellX = a->activeX = 0;
					}

					if (a->activeY - 1 >= 0)
					{
						cellY = a->activeY -= 1;
					}
					else if (activeAnimation->loop)
					{
						cellX = a->activeX = 0;
						cellY = a->activeY = activeAnimation->rows - 1;
					}
				}
			}

			PositionComponent* pos = a->pos;

			if (pos->x + ((activeAnimation->width / activeAnimation->columns) / 2.0f) > Game::main.leftX && pos->x - ((activeAnimation->width / activeAnimation->columns) / 2.0f) < Game::main.rightX &&
				pos->y + ((activeAnimation->height / activeAnimation->rows) / 2.0f) > Game::main.bottomY && pos->y - ((activeAnimation->height / activeAnimation->rows) / 2.0f) < Game::main.topY &&
				pos->z < Game::main.camZ)
			{
				// std::cout << std::to_string(activeAnimation->width) + "/" + std::to_string(activeAnimation->height) + "\n";
				Game::main.renderer->prepareQuad(pos, activeAnimation->width, activeAnimation->height, a->scaleX, a->scaleY, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), activeAnimation->ID, a->mapTex->ID, cellX, cellY, activeAnimation->columns, activeAnimation->rows, a->flippedX, a->flippedY);
			}

		}
	}
}

void AnimationSystem::AddComponent(Component* component)
{
	anims.push_back((AnimationComponent*)component);
}

void AnimationSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < anims.size(); i++)
	{
		if (anims[i]->entity == e)
		{
			AnimationComponent* s = anims[i];
			anims.erase(std::remove(anims.begin(), anims.end(), s), anims.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Particle System

void ParticleSystem::Update(int activeScene, float deltaTime)
{
	float screenLeft = (Game::main.camX - (Game::main.windowWidth * Game::main.zoom / 1.0f));
	float screenRight = (Game::main.camX + (Game::main.windowWidth * Game::main.zoom / 1.0f));
	float screenBottom = (Game::main.camY - (Game::main.windowHeight * Game::main.zoom / 1.0f));
	float screenTop = (Game::main.camY + (Game::main.windowHeight * Game::main.zoom / 1.0f));
	float screenElev = Game::main.camZ;

	for (int i = 0; i < particles.size(); i++)
	{
		ParticleComponent* p = particles[i];

		if (p->active && p->entity->Get_Scene() == activeScene ||
			p->active && p->entity->Get_Scene() == 0)
		{
			if (p->lastTick >= p->tickRate)
			{
				p->lastTick = 0.0f;
				PositionComponent* pos = (PositionComponent*)p->entity->componentIDMap[positionComponentID];
				glm::vec2 pPos = glm::vec2(pos->x + p->xOffset, pos->y + p->yOffset);

				if (pPos.x > screenLeft && pPos.x < screenRight &&
					pPos.y > screenBottom && pPos.y < screenTop)
				{
					float lifetime = p->minLifetime + static_cast<float>(rand()) * static_cast<float>(p->maxLifetime - p->minLifetime) / RAND_MAX;

					ParticleEngine::main.AddParticles(p->number, pPos.x, pPos.y, p->element, lifetime);
				}
			}
			else
			{
				p->lastTick += deltaTime;
			}
		}
	}
}

void ParticleSystem::AddComponent(Component* component)
{
	particles.push_back((ParticleComponent*)component);
}

void ParticleSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < particles.size(); i++)
	{
		if (particles[i]->entity == e)
		{
			ParticleComponent* s = particles[i];
			particles.erase(std::remove(particles.begin(), particles.end(), s), particles.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Image System

void ImageSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < images.size(); i++)
	{
		ImageComponent* img = images[i];

		if (img->active && img->entity->Get_Scene() == activeScene ||
			img->active && img->entity->Get_Scene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)img->entity->componentIDMap[positionComponentID];
			StaticSpriteComponent* sprite = (StaticSpriteComponent*)img->entity->componentIDMap[spriteComponentID];

			glm::vec2 anchorPos;

			if (img->anchor == Anchor::topLeft)
			{
				anchorPos = glm::vec2(Game::main.leftX, Game::main.topY) - glm::vec2(-sprite->sprite->width, sprite->sprite->height);
			}
			else if (img->anchor == Anchor::topRight)
			{
				anchorPos = glm::vec2(Game::main.rightX, Game::main.topY) - glm::vec2(sprite->sprite->width, sprite->sprite->height);;
			}
			else if (img->anchor == Anchor::bottomLeft)
			{
				anchorPos = glm::vec2(Game::main.leftX, Game::main.bottomY) + glm::vec2(sprite->sprite->width, sprite->sprite->height);;
			}
			else // if (img->anchor == Anchor::bottomRight)
			{
				anchorPos = glm::vec2(Game::main.rightX, Game::main.bottomY) + glm::vec2(-sprite->sprite->width, sprite->sprite->height);;
			}

			pos->x = anchorPos.x + img->x;
			pos->y = anchorPos.y + img->y;
		}
	}
}

void ImageSystem::AddComponent(Component* component)
{
	images.push_back((ImageComponent*)component);
}

void ImageSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < images.size(); i++)
	{
		if (images[i]->entity == e)
		{
			ImageComponent* s = images[i];
			images.erase(std::remove(images.begin(), images.end(), s), images.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma endregion

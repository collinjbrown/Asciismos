#ifndef PARTICLEENGINE_H
#define PARTICLEENGINE_H

#include <algorithm>
#include <vector>
#include "game.h"
#include "texture_2D.h"

// Seeing as particles won't interact much with the other parts of the game, I went ahead and moved much of their logic
// out of ecs.cpp. I didn't want it getting overly cluttered, not to mention that the particle system doesn't exactly
// align with the architecture of the rest of the game (perfectly).

enum class Element { aether, fire, necrotic, dust };
struct Particle
{
	float x;
	float y;
	Element element;
	int ticks;
	int lifetime;

	Particle(float x, float y, Element element, int lifetime)
	{
		this->x = x;
		this->y = y;
		this->element = element;
		this->lifetime = lifetime;

		ticks = 0;
	}
};

class ParticleEngine
{
public:
	static ParticleEngine main;
	float lastTick;
	float tickDelay;
	std::vector<Particle*> particles;

	void Init(float tickDelay)
	{
		// Do absolutely fucking nothing.
		lastTick = 0.0f;
		this->tickDelay = tickDelay;
	}

	void AddParticle(float x, float y, Element element, int lifetime)
	{
		Particle* p = new Particle(x, y, element, lifetime);
		particles.push_back(p);
	}

	void AddParticles(int number, float x, float y, Element element, int lifetime)
	{
		for (int i = 0; i < number; i++)
		{
			Particle* p = new Particle(x, y, element, lifetime);
			particles.push_back(p);
		}
	}

	void RemoveParticle(Particle* p)
	{
		particles.erase(std::remove(particles.begin(), particles.end(), p), particles.end());
		delete p;
	}

	void Update(float deltaTime)
	{
		if (lastTick > tickDelay)
		{
			lastTick = 0.0f;

			for (int p = 0; p < particles.size(); p++)
			{
				Particle* particle = particles[p];
				Texture2D* s = Game::main.textureMap["blank"];
				glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

				int r = rand() % 100 + 1;
				float cr = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				if (particle->element == Element::fire ||
					particle->element == Element::necrotic)
				{
					particle->y += 2.0f;

					if (r > 75)
					{
						particle->x += 2.0f;
					}
					else if (r > 50)
					{
						particle->x -= 2.0f;
					}
					else if (r < 10)
					{
						particle->y -= 2.0f;
					}

					if (particle->element == Element::fire)
					{
						color = glm::vec4(1.0f, cr, 0.0f, 1.0f);
					}
					else
					{
						color = glm::vec4(0.5f, cr, 0.5f, 1.0f);
					}
				}
				else if (particle->element == Element::aether)
				{
					color = glm::vec4(0.0f, 0.8f, cr, 1.0f);

					if (r > 70)
					{
						particle->x += 2.0f;
					}
					else if (r > 40)
					{
						particle->x -= 2.0f;
					}
					else if (r < 20)
					{
						particle->y += 2.0f;
					}
				}
				else if (particle->element == Element::dust)
				{
					color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

					if (r > 95)
					{
						particle->x += 2.0f;
					}
					else if (r > 90)
					{
						particle->x -= 2.0f;
					}
					else if (r < 70)
					{
						particle->y += 2.0f;
					}
				}

				if (particle->ticks < particle->lifetime)
				{
					particle->ticks += 1;
					// Game::main.renderer->prepareQuad(glm::vec2(particle->x, particle->y), 1.0f, 1.0f, color, s->ID);
					Game::main.renderer->prepareQuad(glm::vec2(particle->x, particle->y), s->width / 4.0f, s->height / 4.0f, 1.0f, 1.0f, color, s->ID, false, false);
				}
				else
				{
					RemoveParticle(particle);
				}
			}
		}
		else
		{
			lastTick += deltaTime;

			for (int p = 0; p < particles.size(); p++)
			{
				Particle* particle = particles[p];
				Texture2D* s = Game::main.textureMap["blank"];
				glm::vec4 color;

				float cr = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				if (particle->element == Element::fire)
				{
					color = glm::vec4(1.0f, cr, 0.0f, 1.0f);
				}
				else if (particle->element == Element::aether)
				{
					color = glm::vec4(0.0f, 0.8f, cr, 1.0f);
				}
				else if (particle->element == Element::dust)
				{
					color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					color = glm::vec4(0.5f, cr, 0.5f, 1.0f);
				}

				Game::main.renderer->prepareQuad(glm::vec2(particle->x, particle->y), s->width / 4.0f, s->height / 4.0f, 1.0f, 1.0f, color, s->ID, false, false);
			}
		}
	}
};


#endif

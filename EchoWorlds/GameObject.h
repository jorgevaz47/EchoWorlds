#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "Animation.h"

enum class ObjectType
{
	PLAYER,
	ENEMY,
	PROJECTILE,
	LEVEL
};

struct GameObject
{
	ObjectType type;
	glm::vec2 position, velocity, acceleration;
	float direction;
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;

	GameObject()
	{
		type = ObjectType::LEVEL;
		direction = 1;
		position = velocity = acceleration = glm::vec2(0.0f);
		currentAnimation = -1;
		texture = nullptr;
	}
};
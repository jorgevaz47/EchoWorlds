#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "Animation.h"

enum class PlayerState
{
	IDLE,
	RUNNING,
	JUMPING,
};

struct PlayerData
{
	PlayerState state;
	PlayerData()
	{
		state = PlayerState::IDLE;
	}
};

struct LevelData
{
};

struct EnemyData
{
};

union ObjectData
{
	PlayerData player;
	LevelData level;
	EnemyData enemy;
};

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
	ObjectData data;
	glm::vec2 position, velocity, acceleration;
	float direction;
	float maxSpeedX;
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;
	bool dynamic;

	GameObject() : data{.level = LevelData()}
	{
		type = ObjectType::LEVEL;
		direction = 1;
		maxSpeedX = 0;
		position = velocity = acceleration = glm::vec2(0.0f);
		currentAnimation = -1;
		texture = nullptr;
		dynamic = false;
	}
};
// EchoWorlds.cpp : Defines the entry point for the application.

#include "EchoWorlds.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>

#include "GameObject.h"

using namespace std;

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logicalWidth, logicalHeight;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(nullptr))
	{

	}
};

struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	int playerIndex;

	GameState()
	{
		playerIndex = 0; // TODO: Change this when map is loaded
	}
};

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUNNING = 1;
	std::vector<Animation> playerAnimations;

	std::vector<SDL_Texture*> textures;
	SDL_Texture *idleTexture, *runningTexture, *brickTexture, *grassTexture, *groundTexture, *panelTexture;

	SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filePath)
	{
		SDL_Texture* tex = IMG_LoadTexture(renderer, filePath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void loadResources(SDLState& sdlState)
	{
		playerAnimations.resize(5);
		playerAnimations[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnimations[ANIM_PLAYER_RUNNING] = Animation(4, 0.5f);

		idleTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/Idle.png");
		runningTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/Running.png");
		brickTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/tiles/Brick.png");
		grassTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/tiles/Grass.png");
		groundTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/tiles/Ground.png");
		panelTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/tiles/Panel.png");
	}

	void unloadResources()
	{
		for (SDL_Texture* tex : textures)
		{
			SDL_DestroyTexture(tex);
		}
	}
};

bool initializeSDL(SDLState& sdlState);
void cleanup(SDLState& sdlState);
void drawObject(const SDLState& sdlState, GameState& gameState, GameObject& object, float deltaTime);
void update(const SDLState& sdlState, GameState& gameState, Resources& resource, GameObject& object, float deltaTime);
void createTiles(const SDLState& sdlState, GameState& gameState, const Resources& resource);
void collisionResponse(const SDLState& sdlState, GameState& gameState, Resources& resources, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject& objectA, GameObject& objectB, float deltaTime);
void checkCollision(const SDLState& sdlState, GameState& gameState, Resources& resources, GameObject& objectA, GameObject& objectB, float deltaTime);

int main(int argc, char* argv[])
{
	SDLState sdlState;
	sdlState.width = 1600;
	sdlState.height = 900;
	sdlState.logicalWidth = 640;
	sdlState.logicalHeight = 320;

	if (!initializeSDL(sdlState))
	{
		return 1;
	}
	
	// Loading game assets
	Resources res;
	res.loadResources(sdlState);

	// Setting up game data
	GameState gameState;
	createTiles(sdlState, gameState, res);
	uint64_t prevTime = SDL_GetTicks();

	// Main loop flag
	bool isRunning = true;
	while (isRunning)
	{
		// Event handler
		SDL_Event event{0};

		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // Convert to seconds

		// Handle events in queue
		// First check if there are any events to handle
		// Then handle each event in the queue
		// Finally, draw the updated scene
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
				{
					isRunning = false;
					break;
				}
				case SDL_EVENT_WINDOW_RESIZED:
				{
					// Storing new window size values. Currently not in use.
					sdlState.width = event.window.data1;
					sdlState.height = event.window.data2;
					break;
				}
			}
		}

		// Update all game objects
		for (auto& layer : gameState.layers)
		{
			for (GameObject& object : layer)
			{
				update(sdlState, gameState, res, object, deltaTime);
				// Update animation
				if (object.currentAnimation != -1)
				{
					object.animations[object.currentAnimation].step(deltaTime);
				}
			}
		}

		// Perform drawing
		SDL_SetRenderDrawColor(sdlState.renderer, 20, 10, 30, 255);
		SDL_RenderClear(sdlState.renderer);

		// Draw all game objects
		for (auto& layer : gameState.layers)
		{
			for (GameObject& object : layer)
			{
				drawObject(sdlState, gameState, object, deltaTime);
			}
		}

		// Swap buffers
		SDL_RenderPresent(sdlState.renderer);

		// Update previous time
		prevTime = nowTime;
	}

	res.unloadResources();
	cleanup(sdlState);
	return 0;
}

bool initializeSDL(SDLState& sdlState)
{
	// Initializing SDL and validating it initialized correctly
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		return false;
	}

	// Creating the window and validating it was created
	sdlState.window = SDL_CreateWindow("EchoWorlds", sdlState.width, sdlState.height, SDL_WINDOW_RESIZABLE);
	if (!sdlState.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(sdlState);
		return false;
	}

	// Creating the renderer and validating it was created
	sdlState.renderer = SDL_CreateRenderer(sdlState.window, nullptr);
	if (!sdlState.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", nullptr);
		cleanup(sdlState);
		return false;
	}

	// Configuring the presentation of the game
	SDL_SetRenderLogicalPresentation(sdlState.renderer, sdlState.logicalWidth, sdlState.logicalHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	return true;
}

void cleanup(SDLState& sdlState)
{
	SDL_DestroyWindow(sdlState.window);
	SDL_DestroyRenderer(sdlState.renderer);
	SDL_Quit();
}

void drawObject(const SDLState& sdlState, GameState& gameState, GameObject& object, float deltaTime)
{
	const float spriteSize = 32.0f;
	float srcX = object.currentAnimation != -1 ?
		object.animations[object.currentAnimation].currentFrame() * spriteSize : 0.0f;

	SDL_FRect srcRect = { .x = srcX, .y = 0, .w = spriteSize, .h = spriteSize };
	SDL_FRect dstRect = { .x = object.position.x, .y = object.position.y, .w = 32, .h = 32 };

	SDL_FlipMode flipHorizontal = object.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	SDL_RenderTextureRotated(sdlState.renderer, object.texture, &srcRect, &dstRect, 0, nullptr,
							 flipHorizontal);
}

void update(const SDLState& sdlState, GameState& gameState, Resources& resource, GameObject& object, float deltaTime)
{
	if (object.dynamic)
	{
		// Apply gravity
		object.velocity += glm::vec2(0.0f, 500.0f) * deltaTime;
	}
	
	if (object.type == ObjectType::PLAYER)
	{
		float currentDirection = 0;
		if (sdlState.keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (sdlState.keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		if (currentDirection)
		{
			object.direction = currentDirection;
		}

		switch (object.data.player.state)
		{
			case PlayerState::IDLE:
			{
				if (currentDirection)
				{
					object.data.player.state = PlayerState::RUNNING;
					object.texture = resource.runningTexture;
					object.currentAnimation = resource.ANIM_PLAYER_RUNNING;
				}
				else
				{
					// Decelerate to stop
					if (object.velocity.x)
					{
						const float factor = object.velocity.x > 0 ? -1.5f : 1.5f;
						float amount = factor * object.acceleration.x * deltaTime;
						if (std::abs(amount) > std::abs(object.velocity.x))
						{
							object.velocity.x = 0;
						}
						else
						{
							object.velocity.x += amount;
						}
					}
				}
				break;
			}
			case PlayerState::RUNNING:
			{
				if (!currentDirection)
				{
					object.data.player.state = PlayerState::IDLE;
					object.texture = resource.idleTexture;
					object.currentAnimation = resource.ANIM_PLAYER_IDLE;
				}
				break;
			}
		}

		// Add acceleration to velocity
		object.velocity += currentDirection * object.acceleration * deltaTime;
		if (std::abs(object.velocity.x) > object.maxSpeedX)
		{
			object.velocity.x = currentDirection * object.maxSpeedX;
		}
	}

	// Add velocity to position
	object.position += object.velocity * deltaTime;

	// Handle collisions 
	// NOTE: Currently a simple n^2 collision detection is used. Look to optimize this later.
	for (auto& layer : gameState.layers)
	{
		for (GameObject& otherObject : layer)
		{
			if (&object != &otherObject)
			{
				checkCollision(sdlState, gameState, resource, object, otherObject, deltaTime);
			}
		}
	}
}

void collisionResponse(const SDLState& sdlState, GameState& gameState, Resources& resources, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject& objectA, GameObject& objectB, float deltaTime)
{
	// Player collision handling
	if (objectA.type == ObjectType::PLAYER)
	{
		switch (objectB.type)
		{
			case ObjectType::LEVEL:
			{
				if (rectC.w < rectC.h)
				{
					// Horizontal collision
					if (objectA.velocity.x > 0)
					{
						objectA.position.x -= rectC.w;
					}
					else if (objectA.velocity.x < 0)
					{
						objectA.position.x += rectC.w;
					}
					objectA.velocity.x = 0;
				}
				else
				{
					// Vertical collision
					if (objectA.velocity.y > 0)
					{
						objectA.position.y -= rectC.h;
					}
					else if (objectA.velocity.y < 0)
					{
						objectA.position.y += rectC.h;
					}
					objectA.velocity.y = 0;
				}
				break;
			}
		}
	}
}

void checkCollision(const SDLState& sdlState, GameState& gameState, Resources& resources, GameObject& objectA, GameObject& objectB, float deltaTime)
{
	SDL_FRect rectA = {
		.x = objectA.position.x + objectA.collider.x,
		.y = objectA.position.y + objectA.collider.y,
		.w = objectA.collider.w,
		.h = objectA.collider.h
	};

	SDL_FRect rectB = {
		.x = objectB.position.x + objectB.collider.x,
		.y = objectB.position.y + objectB.collider.y,
		.w = objectB.collider.w,
		.h = objectB.collider.h
	};

	SDL_FRect intersection{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &intersection))
	{
		// Found intersection
		collisionResponse(sdlState, gameState, resources, rectA, rectB, intersection, objectA, objectB, deltaTime);
	}
}

void createTiles(const SDLState& sdlState, GameState& gameState, const Resources& resource)
{
	/*
	*	1 - Ground
	*	2 - Panel
	*	3 - Enemy
	*	4 - Player
	*	5 - Grass
	*	6 - Brick
	*/

	short map[MAP_ROWS][MAP_COLS] = {
		0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	const auto createObject = [&sdlState](int r, int c, SDL_Texture* texture, ObjectType type)
	{
		GameObject gameObject;
		gameObject.type = type;
		gameObject.position = glm::vec2(c * TILE_SIZE, sdlState.logicalHeight - (MAP_ROWS - r) * TILE_SIZE);
		gameObject.texture = texture;
		gameObject.collider = SDL_FRect{ .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };
		return gameObject;
	};

	for (int r = 0; r < MAP_ROWS; ++r)
	{
		for (int c = 0; c < MAP_COLS; ++c)
		{
			switch (map[r][c])
			{
				case 1: // Ground tile
				{
					GameObject ground = createObject(r, c, resource.groundTexture, ObjectType::LEVEL);
					gameState.layers[LAYER_IDX_LEVEL].push_back(ground);
					break;
				}
				case 2: // Panel tile
				{
					GameObject panel = createObject(r, c, resource.panelTexture, ObjectType::LEVEL);
					gameState.layers[LAYER_IDX_LEVEL].push_back(panel);
					break;
				}
				case 4: // Player tile
				{
					GameObject player = createObject(r, c, resource.idleTexture, ObjectType::PLAYER);
					player.data.player = PlayerData();
					player.animations = resource.playerAnimations;
					player.currentAnimation = resource.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(300, 0);
					player.maxSpeedX = 100;
					player.dynamic = true;
					player.collider = SDL_FRect{ .x = 11, .y = 6, .w = 10, .h = 26 };
					gameState.layers[LAYER_IDX_CHARACTERS].push_back(player);
					break;
				}	
			}
		}
	}
}
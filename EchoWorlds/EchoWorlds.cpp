﻿// EchoWorlds.cpp : Defines the entry point for the application.

#include "EchoWorlds.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>

#include "Animation.h"

using namespace std;

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logicalWidth, logicalHeight;
};

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0;
	std::vector<Animation> playerAnimations;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* idleTexture;

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

		idleTexture = loadTexture(sdlState.renderer, "EchoWorlds/assets/Idle.png");
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
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float playerX = 150.0f;
	const float floor = sdlState.logicalHeight;
	uint64_t prevTime = SDL_GetTicks();
	bool flipHorizontal = false;

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

		// Handle player movement
		float moveAmount = 0;
		if (keys[SDL_SCANCODE_A])
		{
			moveAmount -= 75.0f;
			flipHorizontal = true;
		}
		if (keys[SDL_SCANCODE_D])
		{
			moveAmount += 75.0f;
			flipHorizontal = false;
		}
		playerX += moveAmount * deltaTime;


		// Perform drawing
		SDL_SetRenderDrawColor(sdlState.renderer, 20, 10, 30, 255);
		SDL_RenderClear(sdlState.renderer);

		const float spriteSize = 32.0f;
		SDL_FRect srcRect = { .x = 0, .y = 0, .w = spriteSize, .h = spriteSize };
		SDL_FRect dstRect = { .x = playerX, .y = floor - spriteSize, .w = 32, .h = 32 };

		SDL_RenderTextureRotated(sdlState.renderer, res.idleTexture, &srcRect, &dstRect, 0, nullptr,
								 (flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

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

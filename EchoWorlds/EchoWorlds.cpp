// EchoWorlds.cpp : Defines the entry point for the application.

#include "EchoWorlds.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

using namespace std;

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
};

void cleanup(SDLState& sdlState);

int main(int argc, char* argv[])
{
	SDLState sdlState;

	// Initializing SDL and validating it initialized correctly
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
	}

	// Creating the window and validating it was created
	int width = 800;
	int height = 600;
	sdlState.window = SDL_CreateWindow("EchoWorlds", width, height, 0);
	if (!sdlState.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(sdlState);
		return 1;
	}

	// Creating the renderer and validating it was created
	sdlState.renderer = SDL_CreateRenderer(sdlState.window, nullptr);
	if (!sdlState.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", nullptr);
		cleanup(sdlState);
		return 1;
	}

	// Main loop flag
	bool isRunning = true;
	while (isRunning)
	{
		// Event handler
		SDL_Event event{0};

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
			}
		}

		// Perform drawing
		SDL_SetRenderDrawColor(sdlState.renderer, 255, 255, 255, 255);
		SDL_RenderClear(sdlState.renderer);

		// Swap buffers
		SDL_RenderPresent(sdlState.renderer);

	}

	cleanup(sdlState);
	return 0;
}

void cleanup(SDLState& sdlState)
{
	SDL_DestroyWindow(sdlState.window);
	SDL_DestroyRenderer(sdlState.renderer);
	SDL_Quit();
}

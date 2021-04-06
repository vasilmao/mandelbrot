#include "App.h"

const int maxn = 255;
const int maxcoord = 100;

inline int GetPointColor(App* app, double x0, double y0) {
	int i = 0;
	double x = x0;
	double y = y0;
	for (; i < maxn; ++i) {
		double x2 = x * x;
		double y2 = y * y;
		double xy = x * y;
		if (x2 + y2 >= maxcoord) {
			break;
		}
		x = x2 - y2 + x0;
		y = 2 * xy + y0;
	}
	return ((255 * (i / maxn) * (i / maxn))) * 256 * 256 + ((255 * (i / maxn) * (i / maxn) * (i / maxn))) * 256 + (255 * sqrt(sqrt((double)i / (double)maxn)));
}

void initSDL(App* app) {
	int rendererFlags, windowFlags;
	rendererFlags = SDL_RENDERER_ACCELERATED;
	windowFlags = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	app->window = SDL_CreateWindow("Patcher", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
	if (!app->window) {
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);
	app->surface = SDL_GetWindowSurface(app->window);
	if (!app->renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
	app->scale = 0.00005;
	app->c_x = SCREEN_WIDTH / 2 * app->scale;
	app->c_y = SCREEN_HEIGHT / 2 * app->scale;
}

void initMand(App* app) {
    app->c_x = 0;
    app->c_y = 0;

}

void ParseEvent(App* app, SDL_Event event) {
    if (event.type == SDL_QUIT) {
        app->running = false;
    }
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
			case SDLK_RIGHT:
				app->c_x -= 50.0 * app->scale;
				break;
			case SDLK_LEFT:
				app->c_x += 50.0 * app->scale;
				break;
			case SDLK_UP:
				app->c_y += 50.0 * app->scale;
				break;
			case SDLK_DOWN:
				app->c_y -= 50.0 * app->scale;
				break;
			case SDLK_LSHIFT:
				app->scale /= 2;
				app->c_x -= SCREEN_WIDTH / 2 * app->scale;
				app->c_y -= SCREEN_HEIGHT / 2 * app->scale;
				break;
			case SDLK_LCTRL:
				app->c_x += SCREEN_WIDTH / 2 * app->scale;
				app->c_y += SCREEN_HEIGHT / 2 * app->scale;
				app->scale *= 2;
		}
	}
}

void MandCycle(App* app) {
    SDL_Event event;
    while (app->running) {
		unsigned int t = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            ParseEvent(app, event);
        }
        PrepareScene(app);
		printf("%lf\n", 1000 / ((double)(SDL_GetTicks() - t)));
		SDL_UpdateWindowSurface(app->window);
        // RenderScene(app);
		break;
    }
}

void PrepareScene(App* app) {
	SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
	SDL_RenderClear(app->renderer);
	unsigned int * pixels = (unsigned int*)app->surface->pixels;
	for (int x = 0; x < SCREEN_WIDTH; ++x) {
		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			double xp = x * app->scale - app->c_x;
			double yp = y * app->scale - app->c_y;
			int color = GetPointColor(app, xp, yp);
			SDL_SetRenderDrawColor(app->renderer, color, 0, 0, 255);
			// SDL_RenderDrawPoint(app->renderer, x, y);
			pixels[x + y * SCREEN_WIDTH] = color;
		}
	}
}

void RenderScene(App* app) {
	SDL_RenderPresent(app->renderer);
}

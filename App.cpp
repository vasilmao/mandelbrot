#include "App.h"

const int maxn = 255;
const double maxcoord = 100;


inline void GetPointColor(App* app, __m256d x0, double y, double* coords_for_compare, unsigned int* color_to_set) {
	// need: mul, mul const, add, sub
	int i = 0;
	// double x = x0;
	int counter[4] = {0};
	__m256d y0 = _mm256_set1_pd(y);
	__m256d x_v = x0;
	__m256d y_v = y0;
	__m256d two_v = _mm256_set1_pd(2);
	char mask = 15;
	char mask_to_add = 0;
	for (; i < maxn; ++i) {
		mask_to_add = 0;
		__m256d x2 = _mm256_mul_pd(x_v, x_v);
		__m256d y2 = _mm256_mul_pd(y_v, y_v);
		__m256d xy = _mm256_mul_pd(x_v, y_v);
		__m256d length = _mm256_add_pd(x2, y2);
		_mm256_storeu_pd(coords_for_compare, length);
		if (mask & 1) {
			if (coords_for_compare[0] >= maxcoord) {
				mask &= (15 - 1);
				counter[0] = i;
			}
		}
		if (mask & 2) {
			if (coords_for_compare[1] >= maxcoord) {
				mask &= (15 - 2);
				counter[1] = i;
			}
		}
		if (mask & 4) {
			if (coords_for_compare[2] >= maxcoord) {
				mask &= (15 - 4);
				counter[2] = i;
			}
		}
		if (mask & 8) {
			if (coords_for_compare[3] >= maxcoord) {
				mask &= (15 - 8);
				counter[3] = i;
			}
		}
		x_v = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
		y_v = _mm256_add_pd(_mm256_mul_pd(xy, two_v), y0);
	}
	if (counter[0] == 0) {
		counter[0] = maxn;
	}
	if (counter[1] == 0) {
		counter[1] = maxn;
	}
	if (counter[2] == 0) {
		counter[2] = maxn;
	}
	if (counter[3] == 0) {
		counter[3] = maxn;
	}
	color_to_set[0] = 255 * sqrt(sqrt((double)counter[0] / (double)maxn));
	color_to_set[1] = 255 * sqrt(sqrt((double)counter[1] / (double)maxn));
	color_to_set[2] = 255 * sqrt(sqrt((double)counter[2] / (double)maxn));
	color_to_set[3] = 255 * sqrt(sqrt((double)counter[3] / (double)maxn));
	// return ((255 * sqrt(sqrt((double)i / (double)maxn)))) * 256;
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
	app->scale = 0.005;
	app->c_x = SCREEN_WIDTH / 2 * app->scale;
	app->c_y = SCREEN_HEIGHT / 2 * app->scale;
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
	printf("%d\n", app->surface->format->BytesPerPixel);
    while (app->running) {
		unsigned int t = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            ParseEvent(app, event);
        }
        PrepareScene(app);
		printf("%lf\n", 1000 / ((double)(SDL_GetTicks() - t)));
		SDL_UpdateWindowSurface(app->window);
        // RenderScene(app);
    }
}

void PrepareScene(App* app) {
	SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
	SDL_RenderClear(app->renderer);
	unsigned int * pixels = (unsigned int*)app->surface->pixels;
	double coords_for_compare[4];
	for (int x = 0; x < SCREEN_WIDTH; x += 4) {
		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			// need: mul const, sub const
			__m256d coords_x = _mm256_set_pd(x + 3, x + 2, x + 1, x);
			__m256d scale_v = _mm256_set1_pd(app->scale);
			double yp = y * app->scale - app->c_y;
			// double yp = y * app->scale - app->c_y;
			coords_x = _mm256_mul_pd(coords_x, scale_v);
			scale_v = _mm256_set1_pd(app->c_x);
			coords_x = _mm256_sub_pd(coords_x, scale_v);
			GetPointColor(app, coords_x, yp, coords_for_compare, pixels + x + y * SCREEN_WIDTH);
			// SDL_SetRenderDrawColor(app->renderer, color, 0, 0, 255);
			// SDL_RenderDrawPoint(app->renderer, x, y);
			// pixels[x + y * SCREEN_WIDTH] = color;
		}
	}
}

void RenderScene(App* app) {
	SDL_RenderPresent(app->renderer);
}

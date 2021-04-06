#include "App.h"

const int maxn = 255;
const double maxcoord = 100;

inline void GetPointColor(App* app, __m256d x0, double y, double* coords_for_compare, unsigned int* color_to_set);
void ParseEvent(App* app, SDL_Event event);

inline void GetPointColor(App* app, __m256d x0, double y, double* coords_for_compare, unsigned int* color_to_set) {
	int i = 0;
	double color_d[4] = {0};
	__m256d y0 = _mm256_set1_pd(y);
	__m256d x_v = x0;
	__m256d y_v = y0;
	__m256d two_v = _mm256_set1_pd(2);
	__m256d out_of_range_v = _mm256_set1_pd(maxcoord);
	__m128i i_counter_v = _mm_setzero_si128();
	for (; i < maxn; ++i) {
		__m256d x2 = _mm256_mul_pd(x_v, x_v);
		__m256d y2 = _mm256_mul_pd(y_v, y_v);
		__m256d xy = _mm256_mul_pd(x_v, y_v);
		__m256d length = _mm256_add_pd(x2, y2);
		__m256d cmp = _mm256_cmp_pd(length, out_of_range_v, 1);
		int mask = _mm256_movemask_pd(cmp);
		if (!mask) {
			break;
		}
		i_counter_v = _mm_sub_epi32 (i_counter_v, _mm256_castsi256_si128(_mm256_castpd_si256(cmp)));
		x_v = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
		y_v = _mm256_add_pd(_mm256_mul_pd(xy, two_v), y0);
	}
	__m256d maxn_v = _mm256_set1_pd(maxn);
	__m256d vec_128 = _mm256_set1_pd(128);
	__m256d vec_255 = _mm256_set1_pd(255);
	__m256d vec_512 = _mm256_set1_pd(512);
	__m256d color_v = _mm256_loadu_pd(color_d);
	__m256d counter_d_v = _mm256_cvtepi32_pd(i_counter_v);
	color_v = _mm256_mul_pd(vec_255, _mm256_div_pd(counter_d_v, maxn_v));
	color_v = _mm256_add_pd(_mm256_mul_pd(color_v, vec_128), _mm256_mul_pd(color_v, vec_512));
	_mm256_storeu_pd(color_d, color_v);
	color_to_set[0] = color_d[0];
	color_to_set[1] = color_d[1];
	color_to_set[2] = color_d[2];
	color_to_set[3] = color_d[3];
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
		break;
    }
}

void PrepareScene(App* app) {
	SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
	SDL_RenderClear(app->renderer);
	unsigned int * pixels = (unsigned int*)app->surface->pixels;
	double coords_for_compare[4];
	for (int x = 0; x < SCREEN_WIDTH; x += 4) {
		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			__m256d coords_x = _mm256_set_pd(x + 3, x + 2, x + 1, x);
			__m256d scale_v = _mm256_set1_pd(app->scale);
			double yp = y * app->scale - app->c_y;
			coords_x = _mm256_mul_pd(coords_x, scale_v);
			__m256d center_x_v = _mm256_set1_pd(app->c_x);
			coords_x = _mm256_sub_pd(coords_x, center_x_v);
			GetPointColor(app, coords_x, yp, coords_for_compare, pixels + x + y * SCREEN_WIDTH);
		}
	}
}

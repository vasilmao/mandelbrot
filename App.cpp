#include "App.h"

const int           MAXITER = 255;
const double        MAXCOORD = 100;
const long long int ALL_F = -1;
const double        start_scale = 0.005;
const double        movement_speed = 50.0;
const __m256d       _v_two = _mm256_set1_pd(2);
const __m256d       _v_MAXITER  = _mm256_set1_pd(MAXITER);
const __m256d       _v_128   = _mm256_set1_pd(128);
const __m256d       _v_255   = _mm256_set1_pd(255);
const __m256d       _v_512   = _mm256_set1_pd(512);
const __m256d       _v_MAXCOORD = _mm256_set1_pd(MAXCOORD);

inline void GetPointColor(App* app, __m256d x, double y, double* coords_for_compare, unsigned int* color_to_set);
void ParseEvent(App* app, SDL_Event event);

inline void GetPointColor(App* app, __m256d x, double y, double* coords_for_compare, unsigned int* color_to_set) {
	int     i               = 0;
	__m256d _v_x0           = x;
	__m256d _v_y0           = _mm256_set1_pd(y);
	__m256d _v_x            = x;
	__m256d _v_y            = _v_y0;
	__m256i _v_iter_counter = _mm256_setzero_si256();
	for (; i < MAXITER; ++i) {
		__m256d _v_x2     = _mm256_mul_pd(_v_x, _v_x);
		__m256d _v_y2     = _mm256_mul_pd(_v_y, _v_y);
		__m256d _v_xy     = _mm256_mul_pd(_v_x, _v_y);
		__m256d _v_length = _mm256_add_pd(_v_x2, _v_y2);
		__m256d _v_cmp_result    = _mm256_cmp_pd(_v_length, _v_MAXCOORD, 1);
		int mask = _mm256_movemask_pd(_v_cmp_result);
		if (!mask) {
			break;
		}
		_v_iter_counter = _mm256_sub_epi64 (_v_iter_counter, _mm256_castpd_si256(_v_cmp_result));
		_v_x = _mm256_add_pd(_mm256_sub_pd(_v_x2, _v_y2), _v_x0);
		_v_y = _mm256_add_pd(_mm256_mul_pd(_v_xy, _v_two), _v_y0);
	}
	double    color_d[4]        = {0};
	long long counter_arr_ll[4] = {0};
	double    counter_arr_d[4]  = {0};

	_mm256_maskstore_epi64(counter_arr_ll, _mm256_set1_epi64x(ALL_F), _v_iter_counter);
	for (int i = 0; i < 4; ++i) counter_arr_d[i] = counter_arr_ll[i]; // conver int64 to double
	__m256d _v_counter_d = _mm256_loadu_pd(counter_arr_d);

	__m256d _v_color = _mm256_mul_pd(_v_255, _mm256_div_pd(_v_counter_d, _v_MAXITER));
	_v_color = _mm256_add_pd(_mm256_mul_pd(_v_color, _v_128), _mm256_mul_pd(_v_color, _v_512));
	_mm256_storeu_pd(color_d, _v_color);

	for (int i = 0; i < 4; ++i) color_to_set[i] = color_d[i];
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
	app->scale = start_scale;
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
				app->c_x -= movement_speed * app->scale;
				break;
			case SDLK_LEFT:
				app->c_x += movement_speed * app->scale;
				break;
			case SDLK_UP:
				app->c_y += movement_speed * app->scale;
				break;
			case SDLK_DOWN:
				app->c_y -= movement_speed * app->scale;
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
        while (SDL_PollEvent(&event)) {
            ParseEvent(app, event);
        }
		unsigned int ticks_before_computing = SDL_GetTicks();
        PrepareScene(app);
		printf("%lf\n", 1000 / ((double)(SDL_GetTicks() - ticks_before_computing)));
		SDL_UpdateWindowSurface(app->window);
		// break;
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

#include <SDL2/SDL.h>
#include <immintrin.h>

#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600

struct App {
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Surface *surface;
    bool running;
    double w;
    double h;
	double scale;
	double c_x;
	double c_y;
};

void PrepareScene(App* app);
void initSDL(App* app);
void MandCycle(App* app);

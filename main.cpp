#include "App.h"

int main() {
    App app = {NULL, NULL, NULL, true};
	initSDL(&app);
    MandCycle(&app);
    SDL_Quit();
    return 0;
}

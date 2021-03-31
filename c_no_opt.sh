##g++ patcher_graphical.cpp App.cpp -lSDL
g++ main.cpp App.cpp -O3 `sdl2-config --libs` -lSDL2_ttf

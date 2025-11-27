#pragma once
#include "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include "GameObject.h"
#include <string>
#include <SDL_ttf.h>
#include <windows.h>  

class Game {
public:
    Game();
    ~Game();

    void init(const char* title, int width, int height, bool fullscreen);
    void handleEvents();
    void update();
    void render();
    void clean();

    TTF_Font* font = nullptr;
    SDL_Color textColor = { 255, 255, 255, 255 };

    SDL_Texture* targetWordTexture = nullptr;
    SDL_Texture* currentInputTexture = nullptr;
    SDL_Texture* feedbackTexture = nullptr; 

    bool running() { return isRunning; }

    void sendStopSignal();

    static SDL_Renderer* renderer;

private:
    bool isRunning = false;
    SDL_Window* window = nullptr;
    std::string getRandomWord();
    std::string targetWord;
    std::string currentInput;

    HANDLE serialHandle = INVALID_HANDLE_VALUE;

};

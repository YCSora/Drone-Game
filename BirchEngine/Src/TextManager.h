#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class TextManager {
public:
    static TTF_Font* LoadFont(const char* path, int fontSize);
    static SDL_Texture* RenderText(const std::string& message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer);
};
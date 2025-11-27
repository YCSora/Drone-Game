#include "TextManager.h"
#include <iostream>

TTF_Font* TextManager::LoadFont(const char* path, int fontSize) {
    TTF_Font* font = TTF_OpenFont(path, fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }
    return font;
}

SDL_Texture* TextManager::RenderText(const std::string& message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    if (message.empty()) {
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surface) {
        std::cerr << "Text surface creation failed: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Text texture creation failed: " << SDL_GetError() << std::endl;
    }

    return texture;
}
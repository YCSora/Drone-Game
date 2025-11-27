#pragma once
#include "Game.h"
#include "TextureManager.h"

class GameObject {
public:
    GameObject(const char* textureSheet, int x, int y);
    ~GameObject() {}

    void Update();
    void Render();

    const float pPerM = 32.0f;
    float gravity = (9.81f * pPerM) / (3600.0f);
    float velocity = 0.0f;

	float idleTime = 0.0f;
	bool gravityActive = false;

    void resetIdleTimer() {
        idleTime = 0.0f;
        gravityActive = false;
        stopSent = false;   
        velocity = 0.0f;
    }

    bool isTakingOff = true;
    float takeoffVelocity = -3.0f;

    int getY() const { 
        return static_cast<int>(ypos); 
    }

    void move(float dy) { 
        ypos += dy; 
    }

    void acceleration(float acc) {
        velocity -= acc;
        if (velocity < -4.0f) velocity = -4.0f;
	}

    float setVelocity(float vel) {
        velocity = vel;
        return vel;
	}

    void impulse(float imp) {
        velocity = imp;
    }

    float getVelocity() const { 
        return velocity; 
    }
    float getGravity() {
        return gravity; 
    } 
private:
    float xpos = 0.0f;
    float ypos = 0.0f;

    bool stopSent = false;

    SDL_Texture* objTexture = nullptr;
    SDL_Rect srcRect{};
    SDL_Rect destRect{};
};
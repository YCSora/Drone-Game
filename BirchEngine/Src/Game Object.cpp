#include "GameObject.h"
#include "Game.h"

// Tell the compiler this global exists in another file
extern Game* game;


GameObject::GameObject(const char* textureSheet, int x, int y) {
    objTexture = TextureManager::LoadTexture(textureSheet);
    xpos = x;
    ypos = y;
}

void GameObject::Update() {
    // setup sprite rects 
    srcRect = { 0, 0, 32, 32 };
    destRect.w = srcRect.w * 2;
    destRect.h = srcRect.h * 2;
    destRect.x = static_cast<int>(xpos);
    destRect.y = static_cast<int>(ypos);


    static bool hasSpawned = false;
    if (!hasSpawned) {
        hasSpawned = true;
        return;
    }

    // Takeoff animation
    if (isTakingOff) {
        ypos += takeoffVelocity;
        takeoffVelocity += 0.05f; // slow ascent
        if (takeoffVelocity >= 0) {
            isTakingOff = false;
            velocity = 0;
        }
    }

// Idle gravity logic
    else {
        idleTime += 1.0f / 60.0f;
        if (idleTime >= 3.0f)
            gravityActive = true;

        if (gravityActive) {
            if (!stopSent) {
                game->sendStopSignal();
                stopSent = true;
            }

            velocity += gravity;
        }

        if (ypos < 0) { ypos = 0; velocity = 0; }
        if (ypos > 536) { ypos = 536; velocity = 0; }

        velocity *= 0.99f;  // slows down motion
        ypos += velocity;
    }
}


void GameObject::Render() {
    SDL_RenderCopy(Game::renderer, objTexture, &srcRect, &destRect);
}

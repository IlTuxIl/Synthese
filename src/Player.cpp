//
// Created by julien on 11/10/17.
//

#include <SDL_timer.h>
#include <window.h>
#include "Player.h"

Player::Player(){
    controller = new KeyboardController(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT);
    transform = Identity();
    lastTime = 0;
}

void Player::update() {
    int time = SDL_GetTicks();

    if(controller->up()) {
        transform = transform * Translation(0, 0, -speed * (time - lastTime));
    }
    if(controller->down()) {
        transform = transform * Translation(0, 0, speed * (time - lastTime));
    }
    if(controller->left()) {
        transform = transform * Translation(-speed * (time - lastTime), 0, 0);
    }
    if(controller->right()) {
        transform = transform * Translation(speed * (time - lastTime), 0, 0);
    }
    if(key_state(SDLK_KP_PLUS)){
        transform = transform * Translation(0, speed * (time - lastTime), 0);
    }
    if(key_state(SDLK_KP_MINUS)){
        transform = transform * Translation(0, -speed * (time - lastTime), 0);
    }
    lastTime = time;
//    std::cout << transform << std::endl;
}

//
// Created by julien on 11/10/17.
//

#ifndef SYNTHESE_PLAYER_H
#define SYNTHESE_PLAYER_H


#include <mat.h>
#include "controller.hpp"
class Player {
public:
    Player();
    void update();
    Transform transform;

private:
    const Controller* controller;
    float speed = 0.005;
    float rotateSpeed = 100;
    int lastTime;
};


#endif //SYNTHESE_PLAYER_H

#include "controller.hpp"

#include "window.h"

bool KeyboardController::up() const {
  return key_state(up_key_) ;
}

bool KeyboardController::down() const {
  return key_state(down_key_) ;
}

bool KeyboardController::left() const {
  return key_state(left_key_) ;
}

bool KeyboardController::right() const {
  return key_state(right_key_) ;
}

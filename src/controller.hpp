#ifndef _MMACHINE_CONTROLLER_HPP_
#define _MMACHINE_CONTROLLER_HPP_

#include <SDL2/SDL_keycode.h>

class Controller {
  public :
    virtual bool up() const = 0 ;
    virtual bool down() const = 0 ;
    virtual bool left() const = 0 ;
    virtual bool right() const = 0 ;
    virtual ~Controller() {}
} ;

class KeyboardController : public Controller {
  public :
    KeyboardController(
        SDL_Keycode up_key,
        SDL_Keycode down_key,
        SDL_Keycode left_key,
        SDL_Keycode right_key
        ) :
      up_key_(up_key),
      down_key_(down_key),
      left_key_(left_key),
      right_key_(right_key)
    {}

    bool up() const ;
    bool down() const ;
    bool left() const ;
    bool right() const ;

  private:
    const SDL_Keycode up_key_ ;
    const SDL_Keycode down_key_ ;
    const SDL_Keycode left_key_ ;
    const SDL_Keycode right_key_ ;
} ;

#endif

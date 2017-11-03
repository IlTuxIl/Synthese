//
// Created by julien on 14/09/17.
//
#include <mesh.h>
#include <orbiter.h>
#include <draw.h>
#include <program.h>
#include "app.h"
#include "Shader.h"
#include "LoadMesh.h"

class tp2 : public App{

public:
    tp2(int width, int height) : App(width, height){}

    void moveCam(){
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            cam.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            cam.move(mx);
        if(key_state(SDLK_UP))         // le bouton du milieu est enfonce
            cam.translation((float) 0 / (float) window_width(), (float) 50 / (float) window_height());
        if(key_state(SDLK_DOWN))         // le bouton du milieu est enfonce
            cam.translation((float) 0 / (float) window_width(), (float) -50 / (float) window_height());
        if(key_state(SDLK_LEFT))         // le bouton du milieu est enfonce
            cam.translation((float) 50 / (float) window_width(), (float) 0 / (float) window_height());
        if(key_state(SDLK_RIGHT))         // le bouton du milieu est enfonce
            cam.translation((float) -50 / (float) window_width(), (float) 0 / (float) window_height());
    }

    bool initFrameBuffer(){
        glGenTextures(1, &depth_buffer);
        glBindTexture(GL_TEXTURE_2D, depth_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_DEPTH_COMPONENT, buffSizeX, buffSizeY, 0,
                     GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA32F, buffSizeX, buffSizeY, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glGenerateMipmap(GL_TEXTURE_2D);

        glGenTextures(1, &normal_buffer);
        glBindTexture(GL_TEXTURE_2D, normal_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA16F, buffSizeX, buffSizeY, 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);

        glGenTextures(1, &pos_buffer);
        glBindTexture(GL_TEXTURE_2D, pos_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA32F, buffSizeX, buffSizeY, 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);

        glGenFramebuffers(1, &frameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_buffer, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, pos_buffer, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_buffer, 0);

        GLenum buffers[]= { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, buffers);

        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        return true;
    }

    void firstRender(){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, buffSizeX, buffSizeY);
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m.renderMesh(cam);
    }

    void drawFirstRender(){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlitFramebuffer(
                0, 0, buffSizeX, buffSizeY,        // rectangle origine dans READ_FRAMEBUFFER
                0, 0, window_width(), window_height(),        // rectangle destination dans DRAW_FRAMEBUFFER
                GL_COLOR_BUFFER_BIT, GL_LINEAR);                        // ne copier que la couleur (+ interpoler)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    void SSR(){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glGenerateMipmap(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal_buffer);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, pos_buffer);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_buffer);

        glViewport(0, 0, window_width(), window_height());
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s.draw(cam, tex, depth_buffer, normal_buffer, pos_buffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int init(){
        buffSizeX = window_width();
        buffSizeY = window_height();
        m.initMeshBuffer("data/Sponza/sponza.obj", cam);

        if(!initFrameBuffer())
            return -1;

        s = Shader("src/Shaders/SSR.glsl", 3);

        glClearColor(0.2f, 0.2f, 0.2f, 1);
        glEnable(GL_DEPTH_TEST);
        glClearDepthf(1.f);
        glDepthFunc(GL_LESS);
        return 0;
    }

    int quit(){
        return 0;
    }

    int render(){

        moveCam();

        if(key_state('r')){
            s.program = read_program("src/Shaders/SSR.glsl");
            program_print_errors(s.program);
        }

        firstRender();

        if(key_state(' '))
            drawFirstRender();
        else
            SSR();

        return 1;
    }

private:
    GLuint frameBuffer;
    GLuint tex;
    GLuint depth_buffer;
    GLuint normal_buffer;
    GLuint pos_buffer;
    LoadMesh m;
    Orbiter cam;
    Shader s;
    int buffSizeX;
    int buffSizeY;

};

int main(){
    tp2 app(1024, 680);
    app.run();
    return 0;
}
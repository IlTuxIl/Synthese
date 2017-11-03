//
// Created by julien on 14/09/17.
//

#ifndef SYNTHESE_DRAW_H
#define SYNTHESE_DRAW_H

#include <string>
#include <vector>
#include <glcore.h>
#include <vec.h>
#include <orbiter.h>

class Shader {
public:
    Shader(){};
    Shader(char* filename, int);
    void draw(const Orbiter& cam);
    void draw(const Orbiter& cam, GLuint, GLuint);
    void draw(Orbiter& cam, GLuint, GLuint, GLuint);
    void draw(Orbiter& cam, GLuint, GLuint, GLuint, GLuint);
    void setVertexArray(GLuint _vao);
    void setVertexArray(const std::vector<Vector>& vec);
    void setVertexArray(const float* vec, int nb);
    GLuint program;
    GLuint vertexArray;
private:
    std::string read(const char* filename);
    int nbVertex;
    GLuint buffer;


};


#endif //SYNTHESE_DRAW_H

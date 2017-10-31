//
// Created by julien on 14/09/17.
//

#include "Shader.h"
#include <fstream>
#include <sstream>
#include <glcore.h>
#include <program.h>
#include <uniforms.h>
#include <window.h>

std::string Shader::read(const char *filename ){
    std::stringbuf source;
    std::ifstream in(filename);
    // verifie que le fichier existe
    if(in.good() == false)
        // affiche une erreur, si le fichier n'existe pas ou n'est pas accessible
        printf("[error] loading program '%s'...\n", filename);
    else
        printf("loading program '%s'...\n", filename);
    // lire le fichier, jusqu'au premier separateur,
    // le caractere '\0' ne peut pas se trouver dans un fichier texte, donc lit tout le fichier d'un seul coup
    in.get(source, 0);
    // renvoyer la chaine de caracteres
    return source.str();
}

Shader::Shader(char *filename, int nbv) {
    program = read_program(filename);
    program_print_errors(program);
    //prepass = read_program("src/Shaders/prePass.glsl");
    glGenVertexArrays(1, &vertexArray);

    glUseProgram(0);
    glBindVertexArray(0);
    nbVertex = nbv;
}

void Shader::setVertexArray(GLuint _vao) {
    glDeleteVertexArrays(1, &vertexArray);
    vertexArray = _vao;
}

void Shader::setVertexArray(const std::vector<Vector>& vec) {

    GLuint size;
    size = vec.size() * sizeof(Vector);
    glDeleteVertexArrays(1, &vertexArray);
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

    size_t offset= 0;

    glBufferSubData(GL_ARRAY_BUFFER, offset, size, &vec[0].x);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shader::setVertexArray(const float *vec, int nb) {
    GLuint size;
    size = nb * sizeof(float);
    glDeleteVertexArrays(1, &vertexArray);
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

    size_t offset= 0;

    glBufferSubData(GL_ARRAY_BUFFER, offset, size, vec);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shader::draw(const Orbiter& cam) {

    Transform M = Identity();
    Transform V = cam.view();
    Transform P = cam.projection(1024, 680, 45);

    Transform MVP = P*V*M;

    glBindVertexArray(vertexArray);
    glUseProgram(program);

    program_uniform(program, "mvpMatrix", MVP);

    glDrawArrays(GL_TRIANGLES, 0, nbVertex);
}

void Shader::draw(const Orbiter& cam, GLuint texture, GLuint dbuffer) {
    glBindVertexArray(vertexArray);
    glUseProgram(program);

    program_use_texture(program, "diffuse_color", 0, texture);
    program_use_texture(program, "depth_buffer", 1, dbuffer);
    glDrawArrays(GL_TRIANGLES, 0, nbVertex);
}

void Shader::draw(Orbiter& cam, GLuint texture, GLuint dbuffer, GLuint nBuffer) {
    glBindVertexArray(vertexArray);
    glUseProgram(program);

    program_uniform(program, "ViewInv", cam.view().inverse());
    program_uniform(program, "View", cam.view());
    program_uniform(program, "ProjInv", cam.projection(window_width(), window_height(), 45).inverse());
    program_uniform(program, "Proj", cam.projection(window_width(), window_height(), 45));
    program_uniform(program, "ViewPortInv", Viewport(window_width(), window_height()).inverse());

    program_use_texture(program, "diffuse_color", 0, texture);
    program_use_texture(program, "depth_buffer", 1, dbuffer);
    program_use_texture(program, "normal_buffer", 2, nBuffer);

    glDrawArrays(GL_TRIANGLES, 0, nbVertex);
}

void Shader::draw(Orbiter& cam, GLuint texture, GLuint dbuffer, GLuint nBuffer, GLuint pBuffer) {
    glBindVertexArray(vertexArray);
    glUseProgram(program);

    program_uniform(program, "ViewPortProj", Viewport(window_width(), window_height()) * cam.projection(window_width(), window_height(), 45));
    program_uniform(program, "View", cam.view().normal());
    program_uniform(program, "invView", cam.view().inverse());

    program_uniform(program, "Proj", cam.projection(window_width(), window_height(), 45));


    program_use_texture(program, "diffuse_color", 0, texture);
    program_use_texture(program, "normal_buffer", 1, nBuffer);
    program_use_texture(program, "pos_buffer", 2, pBuffer);
    program_use_texture(program, "depth_buffer", 3, dbuffer);

    glDrawArrays(GL_TRIANGLES, 0, nbVertex);
}
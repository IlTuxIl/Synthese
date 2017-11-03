//
// Created by julien on 03/11/17.
//

#ifndef SYNTHESE_LOADMESH_H
#define SYNTHESE_LOADMESH_H

#include <string>
#include <orbiter.h>
#include <mesh_buffer.h>

class LoadMesh {
public:
    void initMeshBuffer(std::string filename, Orbiter& cam);
    void renderMesh(Orbiter cam);
private:
    GLuint program;
    GLuint vao;
    int m_vertex_count;
    int m_index_count;
    GLuint m_vertex_buffer;
    GLuint m_index_buffer;
    GLuint m_sampler;
    MeshBuffer m_mesh;
};


#endif //SYNTHESE_LOADMESH_H

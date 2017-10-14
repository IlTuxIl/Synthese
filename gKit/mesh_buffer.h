//! \file mesh_buffer.h representation d'un objet openGL.

#ifndef _MESH_BUFFER_H
#define _MESH_BUFFER_H

#include "mesh_data.h"


//! representation d'une sequence de triangles associes a la meme matiere
struct MeshGroup
{
    int material;       //!< indice de la matiere
    int first;          //!< indice des premiers sommets
    int count;          //!< nombre d'indices
    
    MeshGroup( const int _id= -1, const int _first= 0 ) : material(_id), first(_first), count(0) {}
};


//! representation d'un objet.
struct MeshBuffer
{
    std::vector<vec3> positions;                //!< attribut position
    std::vector<vec2> texcoords;                //!< attribut coordonnees de texture
    std::vector<vec3> normals;                  //!< attribut normale
    
    std::vector<int> material_indices;          //!< indice de la matiere des triangles
    std::vector<int> indices;                   //!< indices des sommets des triangles
    
    std::vector<MaterialData> materials;        //!< ensemble de matieres
    std::vector<MeshGroup> material_groups;     //!< sequence de triangles groupes par matiere
    
    const void *vertex_buffer( ) const { return positions.data(); }
    size_t vertex_buffer_size( ) const { return positions.size() * sizeof(vec3); }
    
    const void *texcoord_buffer( ) const { return texcoords.data(); }
    size_t texcoord_buffer_size( ) const { return texcoords.size() * sizeof(vec2); }
    
    const void *normal_buffer( ) const { return normals.data(); }
    size_t normal_buffer_size( ) const { return normals.size() * sizeof(vec3); }

    const void *index_buffer( ) const { return indices.data(); }
    size_t index_buffer_size( ) const { return indices.size() * sizeof(int); }
    const void *index_buffer_offset( const int first ) const { return (const void *) (first * sizeof(int)); }
    
    const void *material_buffer( ) const { return material_indices.data(); }
    size_t material_buffer_size( ) const { return material_indices.size() * sizeof(int); }
};


//! construction a partir des donnees d'un maillage.
MeshBuffer buffers( const MeshData& data );


#endif

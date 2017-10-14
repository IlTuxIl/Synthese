//! \file mesh_buffer.cpp

#include <cassert>
#include <map>
#include <algorithm>

#include "mesh_data.h"
#include "mesh_buffer.h"


// compare la matiere de 2 triangles
struct compareMaterial
{
    const std::vector<int>& material_buffer;
    
    compareMaterial( const std::vector<int>& _buffer ) : material_buffer(_buffer) {}
    
    bool operator() ( const int& a, const int& b ) const
    {
        return material_buffer[a] < material_buffer[b];
    }
};


//! representation de l'indexation complete d'un sommet
struct MeshVertex
{
    int material;
    int position;
    int texcoord;
    int normal;
    
    MeshVertex( ) : material(-1), position(-1), texcoord(-1), normal(-1) {}
    MeshVertex( const int m, const int p, const int t, const int n ) : material(m), position(p), texcoord(t), normal(n) {}
    
    // comparaison lexicographique de 2 indices
    bool operator< ( const MeshVertex& b ) const
    {
        if(material != b.material) return material < b.material;
        if(position != b.position) return position < b.position;
        if(texcoord != b.texcoord) return texcoord < b.texcoord;
        if(normal != b.normal) return normal < b.normal;
        return false;
    }
};


MeshBuffer buffers( const MeshData& data )
{
    MeshBuffer mesh;

    mesh.materials= data.materials;
    mesh.material_indices.reserve(data.material_indices.size());
    
    mesh.positions.reserve(data.positions.size());
    mesh.texcoords.reserve(data.positions.size());
    mesh.normals.reserve(data.positions.size());
    
    // tri les triangles par matiere
    std::vector<int> triangles;
    triangles.reserve(data.material_indices.size());
    for(int i= 0; i < (int) data.material_indices.size(); i++)
        triangles.push_back(i);
    
    std::sort(triangles.begin(), triangles.end(), compareMaterial(data.material_indices));
    
    // groupes de triangles 
    int material_id= data.material_indices[triangles[0]];
    mesh.material_groups.push_back( MeshGroup(material_id, 0) );
    
    bool has_texcoords= !data.texcoords.empty();
    bool has_normals= !data.normals.empty();
    
    // re ordonne les triangles et les attributs
    std::map<MeshVertex, int> remap;
    for(int i= 0; i < (int) triangles.size(); i++)
    {
        for(int k= 0; k < 3; k++)
        {
            // matiere du triangle
            mesh.material_indices.push_back(data.material_indices[triangles[i]]);
            
            // associe la matiere de la face a ses sommets
            if(material_id != data.material_indices[triangles[i]])
            {
                // construit les groupes de triangles associes a la meme matiere
                material_id= data.material_indices[triangles[i]];
                
                mesh.material_groups.back().count= 3*i - mesh.material_groups.back().first;
                mesh.material_groups.push_back( MeshGroup(material_id, 3*i) );
            }
            // indice du kieme sommet du ieme triangle re-ordonne
            int index= 3*triangles[i] + k;
            MeshVertex vertex= MeshVertex(material_id, data.position_indices[index], data.texcoord_indices[index], data.normal_indices[index]);
            
            auto found= remap.insert( std::make_pair(vertex, remap.size()) );
            if(found.second)
            {
                // copie les attributs
                assert(data.position_indices[index] != -1);
                mesh.positions.push_back( data.positions[data.position_indices[index]] );
                
                if(data.texcoord_indices[index] != -1)
                    // copie les texcoord du sommet, si elles sont definies
                    mesh.texcoords.push_back( data.texcoords[data.texcoord_indices[index]] );
                else if(has_texcoords)
                    // copie une valeur par defaut, tous les sommets n'ont pas de texcoord
                    mesh.texcoords.push_back( vec2() );
                
                if(data.normal_indices[index] != -1) 
                    // copie la normale du sommet, si ell est definie
                    mesh.normals.push_back( data.normals[data.normal_indices[index]] );
                else if(has_normals)
                    // copie une valeur par defaut, tous les sommets n'ont pas de normale
                    mesh.normals.push_back( vec3() );
            }
            
            // construit l'index buffer
            mesh.indices.push_back(found.first->second);
        }
    }

    // termine la description du dernier groupe de triangles
    mesh.material_groups.back().count= 3*triangles.size() - mesh.material_groups.back().first;
    
    printf("buffers : %d positions, %d texcoords, %d normals, %d indices, %d groups\n", 
        (int) mesh.positions.size(), (int) mesh.texcoords.size(), (int) mesh.normals.size(), (int) mesh.indices.size(), (int) mesh.material_groups.size());
    
    
    return mesh;
}

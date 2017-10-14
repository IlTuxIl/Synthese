//! \file mesh_data.h representation des donnees d'un fichier wavefront .obj

#ifndef _MESH_DATA_H
#define _MESH_DATA_H

#include <vector>
#include <string>

#include "glcore.h"
#include "vec.h"
#include "color.h"


//! representation d'une matiere texturee.
struct MaterialData
{
    Color diffuse;                      //!< couleur diffuse
    Color diffuse_texture_color;        //!< couleur moyenne de la texture
    
    std::string diffuse_filename;       //!< nom de la texture diffuse
    GLuint diffuse_texture;             //!< texture diffuse
    
    Color specular;                     //!< couleur du reflet

    Color emission;                     //!< pour une source de lumiere
    float ns;                           //!< exposant pour les reflets blinn-phong
    
    std::string ns_filename;            //!< nom de la texture exposant
    GLuint ns_texture;                  //!< texture exposant
    
    // normal texture... \todo
    
    MaterialData( ) : diffuse(0.8f, 0.8f, 0.8f), diffuse_texture_color(1, 1, 1), diffuse_filename(), diffuse_texture(0), 
        specular(Black()), emission(), 
        ns(0), ns_filename(), ns_texture(0) {}
};

//! ensemble de matieres texturees.
struct MaterialDataLib
{
    std::vector<std::string> names;
    std::vector<MaterialData> data;
};


struct MeshData
{
    std::vector<vec3> positions;
    std::vector<vec2> texcoords;
    std::vector<vec3> normals;
    
    std::vector<int> position_indices;
    std::vector<int> texcoord_indices;
    std::vector<int> normal_indices;
    
    std::vector<MaterialData> materials;
    std::vector<int> material_indices;
};

/*! renvoie le chemin d'acces a un fichier. le chemin est toujours termine par /
    pathname("path\to\file") == "path/to/"
    pathname("path\to/file") == "path/to/"
    pathname("path/to/file") == "path/to/"
    pathname("file") == "./"
 */
std::string pathname( const std::string& filename );

//! charge un fichier wavefront .obj et renvoie les donnees.
MeshData read_mesh_data( const char *filename );

//! charge un ensemble de matieres texturees.
MaterialDataLib read_material_data( const char *filename );

//! renvoie l'englobant.
void bounds( const MeshData& data, Point& pmin, Point& pmax );

//! (re-) calcule les normales des sommets. utiliser avant les reindexations, cf indices() et vertices().
void normals( MeshData& data );

//! construit les sommets. prepare l'affichage openGL, avec glDrawArrays().
MeshData vertices( const MeshData& data );


#endif

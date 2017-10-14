//! \file material_data.h charge les textures utiilisees par un ensemble de matieres.

#ifndef _MATERIAL_DATA_H
#define _MATERIAL_DATA_H

#include "mesh_data.h"


//! charge les textures associees a un ensemble de matieres, sans depasser une limite de taille, 1Go par defaut.
int read_textures( std::vector<MaterialData>& materials, const size_t max_size= 1024*1024*1024 );

//! detruit les textures.
void release_textures( std::vector<MaterialData>& materials );

#endif

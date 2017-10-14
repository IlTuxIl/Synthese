//! \file mesh_data.cpp

#include <cstdio>
#include <ctype.h>
#include <climits>

#include <algorithm>
#include <map>

#include "material_data.h"
#include "mesh_data.h"


/*! renvoie le chemin d'acces a un fichier. le chemin est toujours termine par /
    pathname("path\to\file") == "path/to/"
    pathname("path\to/file") == "path/to/"
    pathname("path/to/file") == "path/to/"
    pathname("file") == "./"
 */
std::string pathname( const std::string& filename )
{
    std::string path= filename;
#ifndef WIN32
    std::replace(path.begin(), path.end(), '\\', '/');   // linux, macos : remplace les \ par /.
    size_t slash = path.find_last_of( '/' );
    if(slash != std::string::npos)
        return path.substr(0, slash +1); // inclus le slash
    else
        return "./";
#else
    std::replace(path.begin(), path.end(), '/', '\\');   // windows : remplace les / par \.
    size_t slash = path.find_last_of( '\\' );
    if(slash != std::string::npos)
        return path.substr(0, slash +1); // inclus le slash
    else
        return ".\\";
#endif
}


MeshData read_mesh_data( const char *filename )
{
    FILE *in= fopen(filename, "rt");
    if(in == NULL)
    {
        printf("[error] loading mesh '%s'...\n", filename);
        return MeshData();
    }
    
    printf("loading mesh '%s'...\n", filename);
    
    MeshData data;
    MaterialDataLib materials;
    int default_material_id= -1;
    int material_id= -1;
    
    std::vector<int> idp;
    std::vector<int> idt;
    std::vector<int> idn;
    
    char tmp[1024];
    char line_buffer[1024];
    bool error= true;
    for(;;)
    {
        // charge une ligne du fichier
        if(fgets(line_buffer, sizeof(line_buffer), in) == NULL)
        {
            error= false;       // fin du fichier, pas d'erreur detectee
            break;
        }
        
        // force la fin de la ligne, au cas ou
        line_buffer[sizeof(line_buffer) -1]= 0;
        
        // saute les espaces en debut de ligne
        char *line= line_buffer;
        while(*line && isspace(*line))
            line++;
        
        if(line[0] == 'v')
        {
            float x, y, z;
            if(line[1] == ' ')          // position x y z
            {
                if(sscanf(line, "v %f %f %f", &x, &y, &z) != 3)
                    break;
                data.positions.push_back( vec3(x, y, z) );
            }
            else if(line[1] == 'n')     // normal x y z
            {
                if(sscanf(line, "vn %f %f %f", &x, &y, &z) != 3)
                    break;
                data.normals.push_back( vec3(x, y, z) );
            }
            else if(line[1] == 't')     // texcoord x y
            {
                if(sscanf(line, "vt %f %f", &x, &y) != 2)
                    break;
                data.texcoords.push_back( vec2(x, y) );
            }
        }
        
        else if(line[0] == 'f')         // triangle a b c, les sommets sont numerotes a partir de 1 ou de la fin du tableau (< 0)
        {
            idp.clear();
            idt.clear();
            idn.clear();
            
            int next;
            for(line= line +1; ; line= line + next)
            {
                idp.push_back(0); 
                idt.push_back(0); 
                idn.push_back(0);         // 0: invalid index
                
                next= 0;
                if(sscanf(line, " %d/%d/%d %n", &idp.back(), &idt.back(), &idn.back(), &next) == 3) 
                    continue;
                else if(sscanf(line, " %d/%d %n", &idp.back(), &idt.back(), &next) == 2)
                    continue;
                else if(sscanf(line, " %d//%d %n", &idp.back(), &idn.back(), &next) == 2)
                    continue;
                else if(sscanf(line, " %d %n", &idp.back(), &next) == 1)
                    continue;
                else if(next == 0)      // fin de ligne
                    break;
            }
            
            // force une matiere par defaut, si necessaire
            if(material_id == -1)
            {
                if(default_material_id == -1)
                {
                    // creer une matiere par defaut
                    default_material_id= data.materials.size();
                    data.materials.push_back( MaterialData() );
                }
                
                material_id= default_material_id;
                printf("usemtl default\n");
            }
            
            // triangule la face, construit les triangles 0 1 2, 0 2 3, 0 3 4, etc
            for(int v= 2; v +1 < (int) idp.size(); v++)
            {
                int idv[3]= { 0, v -1, v };
                for(int i= 0; i < 3; i++)
                {
                    int k= idv[i];
                    int p= (idp[k] < 0) ? (int) data.positions.size() + idp[k] : idp[k] -1;
                    int t= (idt[k] < 0) ? (int) data.texcoords.size() + idt[k] : idt[k] -1;
                    int n= (idn[k] < 0) ? (int) data.normals.size()   + idn[k] : idn[k] -1;
                    
                    if(p < 0 || p >= (int) data.positions.size()) 
                        break; // error
                    
                    // conserve les indices du sommet
                    data.position_indices.push_back(p);
                    data.texcoord_indices.push_back(t);
                    data.normal_indices.push_back(n);
                }
                
                // matiere du triangle...
                data.material_indices.push_back(material_id);
            }
        }
        
        else if(line[0] == 'm')
        {
           if(sscanf(line, "mtllib %[^\r\n]", tmp) == 1)
           {
                materials= read_material_data( std::string(pathname(filename) + tmp).c_str() );
                data.materials= materials.data;
           }
        }
        
        else if(line[0] == 'u')
        {
           if(sscanf(line, "usemtl %[^\r\n]", tmp) == 1)
           {
                material_id= -1;
                for(unsigned int i= 0; i < (unsigned int) materials.names.size(); i++)
                    if(materials.names[i] == tmp)
                        material_id= i;
                
                if(material_id == -1)
                {
                    // force une matiere par defaut, si necessaire
                    if(default_material_id == -1)
                    {
                        // creer une matiere par defaut
                        default_material_id= data.materials.size();
                        data.materials.push_back( MaterialData() );
                    }
                    
                    material_id= default_material_id;
                }
           }
        }
    }
    
    fclose(in);
    
    if(error)
        printf("loading mesh '%s'...\n[error]\n%s\n\n", filename, line_buffer);

    printf("  %d positions, %d texcoords, %d normals, %d triangles\n", 
        (int) data.positions.size(), (int) data.texcoords.size(), (int) data.normals.size(), (int) data.material_indices.size());
    
    return data;
}


static
std::string normalize_path( std::string file )
{
#ifndef WIN32
    std::replace(file.begin(), file.end(), '\\', '/');   // linux, macos : remplace les \ par /.
#else
    std::replace(file.begin(), file.end(), '/', '\\');   // windows : remplace les / par \.
#endif
    return file;
}


MaterialDataLib read_material_data( const char *filename )
{
    MaterialDataLib materials;
    
    FILE *in= fopen(filename, "rt");
    if(in == NULL)
    {
        printf("[error] loading materials '%s'...\n", filename);
        return materials;
    }
    
    printf("loading materials '%s'...\n", filename);
    
    MaterialData *material= NULL;
    std::string path= pathname(filename);
    
    char tmp[1024];
    char line_buffer[1024];
    bool error= true;
    for(;;)
    {
        // charge une ligne du fichier
        if(fgets(line_buffer, sizeof(line_buffer), in) == NULL)
        {
            error= false;       // fin du fichier, pas d'erreur detectee
            break;
        }
        
        // force la fin de la ligne, au cas ou
        line_buffer[sizeof(line_buffer) -1]= 0;
        
        // saute les espaces en debut de ligne
        char *line= line_buffer;
        while(*line && isspace(*line))
            line++;
        
        if(line[0] == 'n')
        {
            if(sscanf(line, "newmtl %[^\r\n]", tmp) == 1)
            {
                materials.names.push_back( tmp );
                materials.data.push_back( MaterialData() );
                material= &materials.data.back();
            }
        }
        
        if(material == NULL)
            continue;
        
        if(line[0] == 'K')
        {
            float r, g, b;
            if(sscanf(line, "Kd %f %f %f", &r, &g, &b) == 3)
                material->diffuse= Color(r, g, b);
            else if(sscanf(line, "Ks %f %f %f", &r, &g, &b) == 3)
                material->specular= Color(r, g, b);
            else if(sscanf(line, "Ke %f %f %f", &r, &g, &b) == 3)
                material->emission= Color(r, g, b);
        }
        
        else if(line[0] == 'N')
        {
            float n;
            if(sscanf(line, "Ns %f", &n) == 1)          // Ns, puissance / concentration du reflet, modele blinn phong
                material->ns= n;
        }
        
        else if(line[0] == 'm')
        {
            if(sscanf(line, "map_Kd %[^\r\n]", tmp) == 1)
                material->diffuse_filename= normalize_path(path + tmp);
            
            if(sscanf(line, "map_Ks %[^\r\n]", tmp) == 1)
                material->ns_filename= normalize_path(path + tmp);
            
            //~ if(sscanf(line, "map_bump %[^\r\n]", tmp) == 1)
                //~ material->normal_filename= tmp;
        }
    }
    
    fclose(in);
    if(error)
        printf("[error] parsing line :\n%s\n", line_buffer);
    
    return materials;
}


void bounds( const MeshData& data, Point& pmin, Point& pmax )
{
    if(data.positions.size() < 1)
        return;
    
    pmin= Point(data.positions[0]);
    pmax= pmin;

    for(int i= 1; i < (int) data.positions.size(); i++)
    {
        vec3 p= data.positions[i];
        pmin= Point( std::min(pmin.x, p.x), std::min(pmin.y, p.y), std::min(pmin.z, p.z) );
        pmax= Point( std::max(pmax.x, p.x), std::max(pmax.y, p.y), std::max(pmax.z, p.z) );
    }
}


void normals( MeshData& data )
{
    // une normale par position
    std::vector<Vector> normals(data.positions.size(), Vector());
    for(int i= 0; i + 2 < (int) data.position_indices.size(); i+= 3)
    {
        // positions des sommets du triangle
        Point a= Point(data.positions[data.position_indices[i]]);
        Point b= Point(data.positions[data.position_indices[i +1]]);
        Point c= Point(data.positions[data.position_indices[i +2]]);
        
        // normale geometrique
        Vector n= normalize(cross(normalize(b - a), normalize(c - a)));
        
        // somme la normale sur les sommets du triangle
        normals[data.position_indices[i]]=    normals[data.position_indices[i]] + n;
        normals[data.position_indices[i +1]]= normals[data.position_indices[i +1]] + n;
        normals[data.position_indices[i +2]]= normals[data.position_indices[i +2]] + n;
    }
    
    // copie 
    data.normals.clear();
    data.normals.reserve(normals.size());
    for(int i= 0; i < (int) normals.size(); i++)
        data.normals.push_back( vec3(normalize(normals[i])) );
    
    // re-indexe les sommets
    for(int i= 0; i < (int) data.normal_indices.size(); i++)
        data.normal_indices[i]= data.position_indices[i];
}


MeshData vertices( const MeshData& data )
{
    MeshData mesh;
    mesh.materials= data.materials;
    mesh.material_indices= data.material_indices;
    
    mesh.positions.reserve(data.positions.size());
    mesh.texcoords.reserve(data.texcoords.size());
    mesh.normals.reserve(data.normals.size());
    
    for(int i= 0; i < (int) data.position_indices.size(); i++)
    {
        mesh.positions.push_back( data.positions[data.position_indices[i]] );
        if(data.texcoord_indices[i]>= 0)
            mesh.texcoords.push_back( data.texcoords[data.texcoord_indices[i]] );
        if(data.normal_indices[i] >= 0)
            mesh.normals.push_back( data.normals[data.normal_indices[i]] );
    }
    
    return mesh;
}


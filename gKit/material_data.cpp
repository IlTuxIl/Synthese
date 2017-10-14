//! \file material_data.cpp 

#include <cstdio>
#include <algorithm>

#include "image_io.h"
#include "texture.h"
#include "material_data.h"
#include "mesh_data.h"


struct TextureData
{
    ImageData diffuse_image;
    bool use_diffuse= false;
    
    ImageData ns_image;
    bool use_ns= false;
};

static
int miplevel_size( const ImageData& image, const int lod )
{
    int w= std::max(1, image.width / (1<<lod));
    int h= std::max(1, image.height / (1<<lod));
    return image.channels * image.size * w * h;
}

static inline
int offset( const int width, const int stride, const int x, const int y, const int c )
{
    return (y * width + x) * stride + c;
}

static
ImageData mipmap_resize( const ImageData& image, const int lod )
{
    assert(image.size == 1);
    
    ImageData level= ImageData(image.width, image.height, image.channels, image.size);
    level.data= image.data;
    
    int w= level.width;
    int h= level.height;
    int stride= level.channels * level.size;
    int row_stride= image.width;        // "preserve" les images de largeur impaire...
    for(int l= 0; l < lod; l++)
    {
        w= std::max(1, w / 2);
        h= std::max(1, h / 2);
        
        for(int y= 0; y < h; y++)
        for(int x= 0; x < w; x++)
        for(int i= 0; i < image.channels; i++)
        {
            int m= 0;
            m= m + level.data[offset(row_stride, stride, 2*x, 2*y, i)];
            m= m + level.data[offset(row_stride, stride, 2*x +1, 2*y, i)];
            m= m + level.data[offset(row_stride, stride, 2*x, 2*y +1, i)];
            m= m + level.data[offset(row_stride, stride, 2*x +1, 2*y +1, i)];
            
            level.data[offset(w, stride, x, y, i)]= m / 4;
        }
        
        row_stride= w;
    }
    
    //~ printf("  resize %d : %dx%d, %dx%d\n", lod, image.width, image.height, w, h);
    
    level.width= w;
    level.height= h;
    return level;
}

static
Color average_color( const ImageData& image )
{
    assert(image.size == 1);

    float color[4]= {0, 0, 0, 0};
    
    int stride= image.channels * image.size;
    for(int y= 0; y < image.height; y++)
    for(int x= 0; x < image.width; x++)
    for(int i= 0; i < image.channels; i++)
        color[i]= color[i] + (float) image.data[offset(image.width, stride, x, y, i)] / 255.f;
    
    return Color(color[0], color[1], color[2], color[3]) / (image.width * image.height);
}


int read_textures( std::vector<MaterialData>& materials, const size_t max_size )
{
    std::vector<TextureData> textures;
    
    // evalue la taille totale occuppee par toutes les images / textures
    size_t total_size= 0;
    for(int i= 0; i < (int) materials.size(); i++)
    {
        const MaterialData &material= materials[i];
        TextureData data;
        
        if(!material.diffuse_filename.empty())
        {
            data.use_diffuse= true;
            data.diffuse_image= read_image_data(material.diffuse_filename.c_str());
            total_size= total_size + data.diffuse_image.width * data.diffuse_image.height * data.diffuse_image.channels * data.diffuse_image.size;
        }
        if(!material.ns_filename.empty())
        {
            data.use_ns= true;
            data.ns_image= read_image_data(material.ns_filename.c_str());
            total_size= total_size + data.ns_image.width * data.ns_image.height * data.ns_image.channels * data.ns_image.size;
        }
        
        if(total_size > max_size)
        {
            // ne stocke plus les images apres avoir depasse la limite de taille
            std::vector<unsigned char>().swap(data.diffuse_image.data);
            std::vector<unsigned char>().swap(data.ns_image.data);
        }
        
        textures.emplace_back(data);
    }
    
    printf("using %dMB / %dMB\n", int(total_size / 1024 / 1024), int(max_size / 1024 / 1024));
    
    // reduit les dimensions des images / textures jusqu'a respecter la limite de taille
    int lod= 0;
    for(; total_size > max_size ; lod++)
    {
        total_size= 0;
        for(int i= 0; i < (int) textures.size(); i++)
        {
            if(textures[i].use_diffuse)
                total_size= total_size + miplevel_size(textures[i].diffuse_image, lod);
            if(textures[i].use_ns)
                total_size= total_size + miplevel_size(textures[i].ns_image, lod);
        }
    }
    
    printf("  lod %d, %dMB\n", lod, int(total_size / 1024 / 1024));
    
    // charge une texture par defaut, en cas d'erreur de chargement
    GLuint default_texture= read_texture(0, "data/grid.png");
    
    printf("resizing textures...\n");
    // construit les textures a la bonne resolution
    for(int i= 0;  i < (int) textures.size(); i++)
    {
        TextureData& data= textures[i]; 
        MaterialData& material= materials[i]; 
        
        if(data.use_diffuse && data.diffuse_image.width > 0)
        {
            if(data.diffuse_image.data.empty())
            {
                // recharge l'image, si necessaire
                data.diffuse_image= read_image_data(material.diffuse_filename.c_str());
                assert(data.diffuse_image.width > 0);
            }
            
            ImageData level= mipmap_resize(data.diffuse_image, lod);
            material.diffuse_texture= make_texture(0, level);
            
            material.diffuse_texture_color= average_color(level);
        }
        else
            material.diffuse_texture= default_texture;
        
        if(data.use_ns && data.ns_image.width > 0)
        {
            if(data.ns_image.data.empty())
            {
                // recharge l'image, si necessaire
                data.ns_image= read_image_data(material.ns_filename.c_str());
                assert(data.ns_image.width > 0);
            }
            
            ImageData level= mipmap_resize(data.ns_image, lod);
            material.ns_texture= make_texture(0, level);
        }
        else
            material.ns_texture= default_texture;
        
        // nettoyage, les images ne sont plus necessaires
        std::vector<unsigned char>().swap(data.diffuse_image.data);
        std::vector<unsigned char>().swap(data.ns_image.data);
    }
    
    return total_size;
}

void release_textures( std::vector<MaterialData>& materials )
{
    for(int i= 0; i < (int) materials.size(); i++)
    {
        const MaterialData& material= materials[i];
        
        if(material.diffuse_texture > 0)
            glDeleteTextures(1, &material.diffuse_texture);
        if(material.ns_texture > 0)
            glDeleteTextures(1, &material.ns_texture);
    }
}


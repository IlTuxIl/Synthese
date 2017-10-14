
#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;

out vec3 vertex_position;
out vec2 vertex_texcoord;
out vec3 vertex_normal;

void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
    
    vertex_position= vec3(mvMatrix * vec4(position, 1));
    vertex_texcoord= texcoord;
    vertex_normal= mat3(mvMatrix) * normal;
}

#endif


#ifdef FRAGMENT_SHADER

uniform vec4 diffuse_color;
uniform sampler2D diffuse_texture;

in vec3 vertex_position;
in vec2 vertex_texcoord;
in vec3 vertex_normal;

out vec4 fragment_color;

void main( )
{
    //~ float cos_theta= max(0, dot(normalize(- vertex_position), normalize(vertex_normal)));
    float cos_theta= abs(dot(normalize(- vertex_position), normalize(vertex_normal)));

#if 0
    // utilise uniquement l'uniform color
    vec3 color= diffuse_color.rgb * cos_theta;
#else
    // ou : utilise la texture
    vec4 color_texture= texture(diffuse_texture, vertex_texcoord);
    if(color_texture.a < 0.3)
        discard;
    
    // module la couleur de la matiere par la couleur de la texture
    vec3 color= diffuse_color.rgb * color_texture.rgb * cos_theta;
#endif
    
    // applique une correction gamma au resultat
    fragment_color= vec4(pow(color, vec3(1.0 / 2, 1.0 / 2, 1.0 / 2)), 1);
    
    //~ fragment_color= vec4(color, 1);
    //~ fragment_color= vec4(abs(vertex_texcoord), 0, 1);
}

#endif

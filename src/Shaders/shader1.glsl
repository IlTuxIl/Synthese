#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 lightPos;
out vec2 vertex_texcoord;

void main( ){
    gl_Position= mvpMatrix * vec4(position,1);
    vertex_position= vec3(mvMatrix * vec4(position, 1));
    vertex_normal= mat3(normalMatrix) * normal;
    lightPos = vec4(0, 0, 5,1).xyz;
    vertex_texcoord = texcoord;
}

#endif

#ifdef FRAGMENT_SHADER


layout(early_fragment_tests) in;

uniform sampler2D diffuse_texture;
uniform sampler2D ns_texture;
uniform vec4 diffuse_color;
uniform float ns;

in vec3 vertex_position;
in vec3 vertex_normal;
in vec3 lightPos;
in vec2 vertex_texcoord;

out vec4 fragment_color;
out vec3 normal_color;
out vec3 pos_color;

float calculReflection(in vec3 eyeDir, in vec3 lightDir, in vec3 normal , in float coeff, in vec3 lightPos, in vec3 vertex_position){
    vec3 h = normalize(-lightDir + eyeDir);
    float reflect = ((coeff+1)/6.24) * pow(max(0, dot(h, normal)),coeff);
    float cos_theta = max(0, dot(normal, normalize(lightPos - vertex_position)));
    return cos_theta * reflect;
}

float calculDiffu(in vec3 vertex_position, in vec3 lightPos, in vec3 normal){
    float cos_theta = max(0, dot(normal, normalize(lightPos - vertex_position)));
    return cos_theta;
}

void main(){

    float lightPower = 500000;
    vec3 eyeDir = normalize(-vertex_position);
    vec4 color = diffuse_color * texture(diffuse_texture, vertex_texcoord);
    vec4 DiffColor = diffuse_color * texture(diffuse_texture, vertex_texcoord);
    vec4 SpecColor = diffuse_color * texture(ns_texture, vertex_texcoord);

    vec3 normal = normalize(vertex_normal);
    float attDist = 1; //lightPower/(pow(length(lightPos - vertex_position), 2));

    float reflect = calculReflection(eyeDir, vertex_position - lightPos, normal, ns, lightPos, vertex_position);
    float diffu = calculDiffu(vertex_position, lightPos, normal);
    color = ((diffu * 1) * DiffColor + (reflect * 0) * SpecColor);

    fragment_color = vec4(pow(color.xyz, vec3(1.0 / 2, 1.0 / 2, 1.0 / 2)), 1);
    normal_color = vertex_normal;
    pos_color = vertex_position;

}

#endif
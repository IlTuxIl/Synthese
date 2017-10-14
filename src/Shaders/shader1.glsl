#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;

layout(location=1) in vec2 texcoord;
out vec2 vertex_texcoord;

uniform mat4 mvpMatrix;

uniform mat4 mvMatrix;
out vec3 vertex_position;

layout(location= 2) in vec3 normal;
uniform mat4 normalMatrix;
out vec3 vertex_normal;
out vec3 lightPos;

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

in vec2 vertex_texcoord;
uniform sampler2D diffuse_color;

in vec3 vertex_position;
in vec3 vertex_normal;

uniform vec4 mesh_color;

out vec4 fragment_color;
in vec3 lightPos;

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

    float lightPower = 5000;
    vec3 eyeDir = normalize(-vertex_position);
    vec4 color = mesh_color * texture(diffuse_color, vertex_texcoord);
    vec3 normal = normalize(vertex_normal);
    float attDist = lightPower/(pow(length(lightPos - vertex_position), 2));

    float reflect = calculReflection(eyeDir, vertex_position - lightPos, normal, 10, lightPos, vertex_position);
    float diffu = calculDiffu(vertex_position, lightPos, normal);

    float a = pow(attDist * ((diffu * 1) + (reflect * 0)), 1/2.2);
    fragment_color = vec4(color.rgb * a, 1);
}

#endif
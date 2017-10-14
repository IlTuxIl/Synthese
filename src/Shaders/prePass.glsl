#version 330
#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;

uniform mat4 mvpMatrix;

uniform mat4 mvMatrix;

layout(location= 2) in vec3 normal;
uniform mat4 normalMatrix;

void main() {
    gl_Position= mvpMatrix * vec4(position,1);
}

#endif
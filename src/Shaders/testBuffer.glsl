#version 330

#ifdef VERTEX_SHADER

uniform mat4 mvpMatrix;
out vec2 vertex_texcoord;

void main() {
    gl_Position = mvpMatrix * ivec4(0,0,0,1);
    if(gl_VertexID == 0){
        gl_Position = vec4(-1,3,-1,1);
        vertex_texcoord = (vec2(0,2));
    }
    if(gl_VertexID == 1){
        gl_Position = vec4(-1,-1,-1,1);
        vertex_texcoord = (vec2(0,0));
    }
    if(gl_VertexID == 2){
        gl_Position = vec4(3,-1,-1,1);
        vertex_texcoord = (vec2(2,0));
    }
}

#endif


#ifdef FRAGMENT_SHADER

in vec2 vertex_texcoord;
uniform sampler2D diffuse_color;
uniform sampler2D depth_buffer;
out vec4 fragment_color;
uniform mat4 projinv;
uniform mat4 viewportinv;

void main( ){
    vec4 color = textureLod(depth_buffer, vertex_texcoord, 0);
    color = projinv * viewportinv * vec4(gl_FragCoord.xy, color.r, 1);

//    if(color.r != 1)
//        fragment_color= vec4(1,1,0,1);
//    else
//    color.xyz = color.xyz / color.w;
    fragment_color= vec4(0,0,-color.z,1);
}
#endif
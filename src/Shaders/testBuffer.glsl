#version 330

#ifdef VERTEX_SHADER

uniform mat4 mvpMatrix;

out vec2 vertex_texcoord;
out vec3 lightPos;

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
    lightPos = vec4(0, 0, 5,1).xyz;
}

#endif


#ifdef FRAGMENT_SHADER

uniform sampler2D diffuse_color;
uniform sampler2D normal_buffer;
uniform sampler2D pos_buffer;
uniform sampler2D depth_buffer;

uniform mat4 View;
uniform mat4 invView;
uniform mat4 Proj;
uniform mat4 ViewPortProj;

in vec3 lightPos;
in vec2 vertex_texcoord;

out vec4 fragment_color;

int maxIter = 250;
const int numBinarySearchSteps = 20;

void BinarySearch(vec3 dir, inout vec3 cur){
    float depth;
    vec4 projectedCoord;

    for(int i = 0; i < numBinarySearchSteps; i++){

        vec4 projectedCoord = ViewPortProj * vec4(cur, 1.0);
        vec2 coor = projectedCoord.xy /= projectedCoord.w;

        depth = texelFetch(pos_buffer, ivec2(coor.xy), 0).z;

        float dDepth = cur.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            cur += dir;
        else
            cur -= dir;
    }
}

int RayMarch(vec3 dir, inout vec3 cur){

    dir *= 0.1;

    vec4 projectedCoord = ViewPortProj * vec4(cur, 1.0);
    vec2 coor = projectedCoord.xy /= projectedCoord.w;

//    float baseDepth = texelFetch(pos_buffer, ivec2(coor.xy), 0).r;

    for(int pas = 0; pas < maxIter; pas++){

        cur += dir;

        projectedCoord = ViewPortProj * vec4(cur, 1.0);
        coor = projectedCoord.xy /= projectedCoord.w;

        float curDepth = texelFetch(pos_buffer, ivec2(coor.xy), 0).z;

        if(cur.z < curDepth){
            BinarySearch(dir, cur);
            return pas;
        }
    }
    return maxIter;
}

void main( ){

    vec4 baseColor = textureLod(diffuse_color, vertex_texcoord, 0);//texture(diffuse_color, vertex_texcoord);
    float depth = textureLod(depth_buffer, vertex_texcoord, 0).r;
    vec3 normal = /*mat3(View) * */textureLod(normal_buffer, vertex_texcoord, 0).xyz;
    vec3 orig = textureLod(pos_buffer, vertex_texcoord, 0).xyz;

    vec3 dir = normalize(reflect(normalize(orig), normalize(normal)));
    vec3 reflectCoord = orig;

    int pas = RayMarch(/*2 * normal + */dir * max(0.1, -orig.z), reflectCoord);
    if(depth < 1 && pas != maxIter){

        vec4 projectedCoor = ViewPortProj * vec4(reflectCoord, 1.0);
        vec2 coor = projectedCoor.xy /= projectedCoor.w;

        vec3 normalPoint = /*mat3(View) * */texelFetch(normal_buffer, ivec2(coor), 0).xyz;
        float cos_theta = max(0, dot(normalize(normalPoint), normalize(orig - reflectCoord)));

        fragment_color = baseColor + vec4(texelFetch(diffuse_color, ivec2(coor), 0).rgb, 1) * cos_theta;
//        fragment_color = vec4(float(pas)/float(maxIter), float(pas)/float(maxIter) ,float(pas)/float(maxIter), 1);
    }
    else
        fragment_color = baseColor;
//        fragment_color = vec4(0,0,0,1);
//        fragment_color = vec4(orig.x, orig.x, orig.x, 1);

}

#endif
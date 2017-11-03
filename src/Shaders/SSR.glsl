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

const int maxIter = 100;
const int numBinarySearchSteps = 100;

float getViewDepth(vec3 viewCoord){
    vec4 projectedCoord = ViewPortProj * vec4(viewCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    return texelFetch(pos_buffer, ivec2(projectedCoord.xy), 0).z;
}

float getDepth(vec3 viewCoord){
    vec4 projectedCoord = ViewPortProj * vec4(viewCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    return texelFetch(depth_buffer, ivec2(projectedCoord.xy), 0).r;
}

int checkSortie(vec3 viewCoord){
    ivec2 texSize = textureSize(diffuse_color, 0);
    vec4 projectedCoord = ViewPortProj * vec4(viewCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;

    if(projectedCoord.x > texSize.x || projectedCoord.x < 0.0 || projectedCoord.y > texSize.y || projectedCoord.y < 0.0)
        return 0;
    else
        return 1;
}

int BinarySearch(vec3 dir, inout vec3 cur){
    float curDepth;
    for(int i = 0; i < numBinarySearchSteps; i++){
        curDepth = getViewDepth(cur);
        if(checkSortie(cur) == 0)
            return 0;
        if(cur.z > curDepth){
            return 1;
        }
        else
            cur -= dir * 1/numBinarySearchSteps;
    }
    return 1;
}

int RayMarch(vec3 dir, inout vec3 cur){

    dir *= 0.1;

    for(int pas = 0; pas < maxIter; pas++){

        cur += dir;
        float curDepth = getViewDepth(cur);

        if(cur.z < curDepth){
            int test = BinarySearch(dir, cur);
            if(test == 1/* && cur.z < curDepth*/)
                return pas;
            else
                return maxIter;
        }
    }
    return maxIter;
}

void main( ){

    vec4 baseColor = textureLod(diffuse_color, vertex_texcoord, 0);
    float depth = textureLod(depth_buffer, vertex_texcoord, 0).r;
    vec3 normal = textureLod(normal_buffer, vertex_texcoord, 0).xyz;
    vec3 orig = textureLod(pos_buffer, vertex_texcoord, 0).xyz;

    vec3 dir = normalize(reflect(normalize(orig), normalize(normal)));
    vec3 reflectCoord = orig;

    int pas = RayMarch(/*2 * normal + */dir * max(0.1, -orig.z), reflectCoord);
    if(depth < 1 && pas != maxIter){

        vec4 projectedCoor = ViewPortProj * vec4(reflectCoord, 1.0);
        vec2 coor = projectedCoor.xy /= projectedCoor.w;

        vec3 normalPoint = texelFetch(normal_buffer, ivec2(coor), 0).xyz;
        float cos_theta = max(0, dot(normalize(normalPoint), normalize(orig - reflectCoord)));

        fragment_color = baseColor + (vec4(texelFetch(diffuse_color, ivec2(coor), 0).rgb, 1) * cos_theta) * 0.3;
//        fragment_color = vec4(float(pas)/float(maxIter), float(pas)/float(maxIter) ,float(pas)/float(maxIter), 1);
    }
    else
        fragment_color = baseColor;

}

#endif
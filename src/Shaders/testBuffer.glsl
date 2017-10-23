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
uniform mat4 Proj;
uniform mat4 ViewPortProj;

in vec3 lightPos;
in vec2 vertex_texcoord;

out vec4 fragment_color;

int maxIter = 400;

int RayMarch(vec3 dir, inout vec3 cur){

    dir *= 1;

    for(int pas = 0; pas < maxIter; pas++){
        cur += dir;

        vec4 projectedCoord = Proj * vec4(cur, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        if(cur.z < texelFetch(pos_buffer, ivec2(projectedCoord.xy), 0).z){
            return pas;
        }
    }
    return maxIter;
}

void main( ){

    vec4 baseColor = texelFetch(diffuse_color, ivec2(gl_FragCoord.xy), 0);//texture(diffuse_color, vertex_texcoord);
    float depth = textureLod(depth_buffer, vertex_texcoord, 0).r;
    vec3 normal = mat3(View) * textureLod(normal_buffer, vertex_texcoord, 0).xyz;
    vec3 orig = textureLod(pos_buffer, vertex_texcoord, 0).xyz;

    vec3 dir = normalize(reflect(normalize(-orig), normalize(normal)));
    vec3 reflectCoord = orig;

    int pas = RayMarch(dir, reflectCoord);
    if(depth < 1 && pas != maxIter){

        vec4 coor = Proj * vec4(reflectCoord, 1.0);
        coor.xy /= coor.w;
        coor.xy = coor.xy * 0.5 + 0.5;


//        vec3 coor = mat3(ViewPortProj) * reflectCoord;
//        vec3 tmp = coor.xyz/coor.w;
//        vec3 normalPoint = mat3(View) * texelFetch(normal_buffer, ivec2(tmp.xy), 0).xyz;
//        float cos_theta = max(0, dot(normalPoint, normalize(reflectCoord - orig)));
        //fragment_color = vec4(float(pas)/float(maxIter), float(pas)/float(maxIter) ,float(pas)/float(maxIter), 1)
        fragment_color = baseColor + vec4(textureLod(diffuse_color, coor.xy, 0).rgb, 1);// * cos_theta;
//        fragment_color = vec4(1);
    }
    else
//    fragment_color = vec4(0,0,0,1);
        fragment_color = baseColor;

//    fragment_color = vec4(abs(normal), 0);
}
#endif
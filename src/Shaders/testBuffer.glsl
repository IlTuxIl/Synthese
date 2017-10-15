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
uniform sampler2D depth_buffer;
uniform sampler2D normal_buffer;
uniform sampler2D pos_buffer;

uniform mat4 View;
uniform mat4 ViewInv;
uniform mat4 Proj;
uniform mat4 ProjInv;
uniform mat4 ViewPortInv;

in vec3 lightPos;
in vec2 vertex_texcoord;

out vec4 fragment_color;

vec4 RayMarch(vec3 dir, inout vec3 hitCoord){

    int cpt = 0;
    int nbPas = 100;
    float pas = 0.1;
    float depth;
    vec4 projectedCoord;

    dir *= pas;
    for(int i = 0; i < nbPas; i++)
    {
        hitCoord += dir;

        projectedCoord = Proj * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        depth = texture(pos_buffer, projectedCoord.xy, 2).z;

        float dDepth = hitCoord.z - depth;

        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
                return vec4(projectedCoord.xy, depth, 0.0);
        }
    }
    return vec4(vertex_texcoord.xy, depth, 0.0);
}

void main( ){

    vec4 baseColor = texture(diffuse_color, vertex_texcoord);
    float depth = texture(depth_buffer, vertex_texcoord).r;
    vec3 normal = (texture(normal_buffer, vertex_texcoord) * View).xyz;

//    vec4 origHomo = ProjInv * ViewPortInv * vec4(gl_FragCoord.xy, 1, 1);
//    vec3 orig = vec3(origHomo.xy / origHomo.w, depth);
    vec3 orig = (texture(pos_buffer, vertex_texcoord)).xyz;
    vec3 dir = normalize(reflect(normalize(orig), normalize(normal)));

    vec4 coords = RayMarch(dir, orig);

    //fragment_color = vec4(baseColor.rgb + 0.3 * texture(diffuse_color, coords.xy).rgb, 1);//vec4(orig.rgb, 1);
    if(orig.z > 1)
        fragment_color = vec4(1);
    else
        fragment_color = vec4(0,0,0,1);
//    fragment_color = vec4(orig, 1);
}
#endif
#version 330

#ifdef VERTEX_SHADER

uniform mat4 mvpMatrix;

out vec3 lightPos;

void main() {
    gl_Position = mvpMatrix * ivec4(0,0,0,1);
    if(gl_VertexID == 0){
        gl_Position = vec4(-1,3,-1,1);
    }
    if(gl_VertexID == 1){
        gl_Position = vec4(-1,-1,-1,1);
    }
    if(gl_VertexID == 2){
        gl_Position = vec4(3,-1,-1,1);
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
uniform mat4 invViewPortProj;

in vec3 lightPos;

out vec4 fragment_color;

const int maxIter = 50;
const int numBinarySearchSteps = 10;
const int nbRayon = 5;
const float or = (sqrt(5) + 1) / 2;
const float pi = 3.14;

float getViewDepth(vec3 viewCoord){
    vec4 projectedCoord = ViewPortProj * vec4(viewCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    return texelFetch(pos_buffer, ivec2(projectedCoord.xy), 0).z;
}

float getDepth(vec3 viewCoord){
    vec4 projectedCoord = ViewPortProj * vec4(viewCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;

    vec4 tmp = invViewPortProj * vec4(projectedCoord.xy, texelFetch(depth_buffer, ivec2(projectedCoord.xy), 0).x, 1);

    return tmp.z/tmp.w;
}

vec3 projDroite(vec3 orig3D, vec3 end3D, out vec3 end2D){
    vec4 projectedCoord = ViewPortProj * vec4(orig3D, 1.0);
    vec3 ret = vec3(projectedCoord.xy / projectedCoord.w, orig3D.z);

    projectedCoord = ViewPortProj * vec4(end3D, 1.0);
    end2D = vec3(projectedCoord.xy / projectedCoord.w, end3D.z);

    return ret;
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

int checkSortieDiscret(vec3 imgCoord){
    ivec2 texSize = textureSize(diffuse_color, 0);

    if(imgCoord.x > texSize.x || imgCoord.x < 0.0 || imgCoord.y > texSize.y || imgCoord.y < 0.0)
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

int BinarySearchDiscret(vec3 dir, inout vec3 cur){
	float curDepth;
	dir /= 5;
	for(int i = 0; i < maxIter; i++){

		curDepth = texelFetch(pos_buffer, ivec2(cur.xy), 0).z;
		if(checkSortieDiscret(cur) == 0)
		    return 0;
		if(cur.z > curDepth)
			return 1;
		else
			cur -= dir * i;
	}
	return 1;
}

int RayMarch(vec3 dir, inout vec3 cur){

    dir *= 0.1;

    float baseDepth = getDepth(cur);

    for(int pas = 0; pas < maxIter; pas++){

        cur += dir;
        float curDepth = getViewDepth(cur);
//        float curDepth = getDepth(cur);

        if(cur.z < curDepth){
//        if(baseDepth > curDepth){
            int test = BinarySearch(dir, cur);
//            int test = BinarySearch2(dir, cur, curDepth);
            if(test == 1)
                return pas;
            else
                return maxIter;
        }
    }
    return maxIter;
}

vec3 rayMarchDiscret(vec3 orig, vec3 end){

    vec3 dir = end-orig;
    float n;
    if(abs(dir.x) > abs(dir.y)){
        n = dir.x;
        dir.xyz /= dir.x;
    }
    else{
        n = dir.y;
        dir.xyz /= dir.y;
    }

    n /= 5;
    dir *= 5;

//    fragment_color = vec4(abs(dir.xy), 0, 1);

    float baseDepth = texelFetch(depth_buffer, ivec2(orig.xy), 0).r;

    for(int i = 0; i < n; i++){
        vec3 cur = orig + dir * i;
        vec3 cur2 = orig + dir * (i + 0.5);

        float curDepth = texelFetch(pos_buffer, ivec2(cur.xy), 0).z;
        float curDepth2 = texelFetch(pos_buffer, ivec2(cur2.xy), 0).z;

        if(cur.z >= 0 || checkSortieDiscret(cur) == 0)
            return vec3(0);

        if(cur.z < curDepth){
			if(BinarySearchDiscret(dir, cur) == 0)
			    return vec3(0);
            return cur;
        }
        else if(cur2.z < curDepth2){
			if(BinarySearchDiscret(dir, cur2) == 0)
			    return vec3(0);
            return cur2;
        }
    }

    return vec3(0);
}

vec3 getAngleFibo(int i){
    float angle = 1 - (2*i+1) / (2 * nbRayon);

    return vec3(cos(or) * sin(angle), sin(or) * sin(angle), cos(angle));
}

void main( ){

    vec4 baseColor = texelFetch(diffuse_color, ivec2(gl_FragCoord.xy), 0);
    float depth = texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).r;
    vec3 normal = texelFetch(normal_buffer, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 orig = texelFetch(pos_buffer, ivec2(gl_FragCoord.xy), 0).xyz;

    for(int i = 0; i < nbRayon; i++){
        vec3 dir = getAngleFibo(i);
    }

    vec3 dir = normalize(reflect(normalize(orig), normalize(normal)));
    vec3 reflectCoord = orig;

    int pas = RayMarch(dir * max(0.1, -orig.z), reflectCoord);
    if(depth < 1 && pas != maxIter){

        vec4 projectedCoor = ViewPortProj * vec4(reflectCoord, 1.0);
        vec2 coor = projectedCoor.xy /= projectedCoor.w;

        vec3 normalPoint = texelFetch(normal_buffer, ivec2(coor), 0).xyz;
        float cos_theta = max(0, dot(normalize(normalPoint), normalize(orig - reflectCoord)));

        fragment_color = baseColor + (vec4(texelFetch(diffuse_color, ivec2(coor), 0).rgb, 0) * cos_theta) * 0.3;
//        fragment_color = vec4(float(pas)/float(maxIter), float(pas)/float(maxIter) ,float(pas)/float(maxIter), 1);
    }
    else
        fragment_color = baseColor;
//        fragment_color = vec4(0,0,0,1);
}

#endif

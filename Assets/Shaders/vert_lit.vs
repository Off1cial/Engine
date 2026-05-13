#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in vec3 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec4 vColour;
out vec3 vNormal;
out vec2 vUV;
out vec3 vPos;

out mat3 vTBN;

void main()
{
    gl_Position =
        uProj *
        uView *
        uModel *
        vec4(aPos, 1.0);

    vColour = vec4(aColour, 1.0);
    vUV = aUV;
    vNormal  = mat3(transpose(inverse(uModel))) * aNormal;
    vPos = vec3(uModel * vec4(aPos, 1.0));

    
    vec3 T = normalize(mat3(uModel) * aTangent);
    vec3 N = normalize(mat3(uModel) * aNormal);

    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    vTBN = mat3(T, B, N);
    
}
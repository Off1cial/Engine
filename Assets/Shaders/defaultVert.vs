#version 330 core


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;





out vec4 vColour;
out vec2 vUV;

void main(){
  gl_Position = uProj *  uView * uModel * vec4(aPos, 1.0);
  vColour = vec4(aColour, 1.0);
  vUV = aUV;
}

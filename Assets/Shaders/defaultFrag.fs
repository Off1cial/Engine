#version 330 core

in vec4 vColour;
in vec2 vUV;
out vec4 FragColour;




uniform vec4 uColour;



void main(){
  //vec4 finalColour = uUseVertexCol ? vColour * uColour : vec4(1.0);
  //vec4 texColour = uUseTexture ? texture(uTexture, vUV) : vec4(1.0);
 // if (uUseVertexCol_exclusive){
   // finalColour = vColour;
 // }
 // FragColour = finalColour * texColour;
  FragColour = vColour;
}

#version 330 core

in vec4 vColour;
in vec2 vUV;

out vec4 FragColour;

uniform vec4 uColour;

uniform bool uUseTexture;
uniform bool uUseVertexCol;
uniform bool uUseVertexCol_exclusive;

uniform sampler2D uTexture;

void main()
{
    vec4 finalColour = uColour;

    if (uUseVertexCol){
        finalColour *= vColour;
    }

    if (uUseVertexCol_exclusive){
        finalColour = vColour;
    }

    vec4 texColour = vec4(1.0);

    if (uUseTexture){
        texColour = texture(uTexture, vUV);
    }

    FragColour = finalColour * texColour;
}
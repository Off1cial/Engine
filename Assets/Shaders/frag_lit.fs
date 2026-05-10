#version 330 core

in vec4 vColour;
in vec3 vNormal;
in vec2 vUV;
in vec3 vPos;


out vec4 FragColour;



uniform vec4 uColour;
uniform bool uUseTexture;
uniform bool uUseVertexColour;
uniform sampler2D uTexture;


struct Light{
  vec3 position;
  vec3 direction;

  vec3 colour;

  float intensity;
  float radius;

  int type;
};

#define MAX_LIGHTS 8

uniform Light uLights[MAX_LIGHTS];
uniform int uLightCount;


void main()
{

  vec3 normal = normalize(vNormal);
  vec3 lighting = vec3(0.15);

  for (int i = 0; i < uLightCount; i++)
  {
    Light light = uLights[i];
    // directional
    if (light.type == 0){
      vec3 lightDir = normalize(light.direction);


      float diff =
      max(dot(normal, lightDir), 0.0);

      lighting +=
        light.colour *
        diff *
        light.intensity;
    }

    if (light.type == 1)
    {
      vec3 toLight = light.position - vPos;

      float dist = length(toLight);

      if (dist < light.radius)
      {
        vec3 lightDir = toLight / dist;

        float diff = max(dot(normal, lightDir), 0.0);

        float attenuation =
            1.0 - (dist / light.radius);

        attenuation *= attenuation; // smoother falloff

        lighting +=
          light.colour *
          diff *
          light.intensity *
          attenuation;
      }
    }
  }


  vec4 finalColour = uColour;

  if (uUseVertexColour){
      finalColour *= vColour;
  }

  vec4 texColour = vec4(1.0);

  if (uUseTexture){
      texColour = texture(uTexture, vUV);
  }

  //FragColour = finalColour * texColour;

  vec3 finalRGB =
    finalColour.rgb *
    texColour.rgb *
    lighting;

  FragColour = vec4(finalRGB, finalColour.a);
}
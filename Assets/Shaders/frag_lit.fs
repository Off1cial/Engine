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

uniform sampler2D uNormalMap;
uniform bool uUseNormalMap;

in mat3 vTBN;

uniform vec3 uViewPos;
uniform float uSpecular;
uniform float uShininess;

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 colour;
    float intensity;
    float radius;
    float cutoff;
    int type;
};

#define MAX_LIGHTS 8
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT       1
#define LIGHT_SPOT        2

uniform Light uLights[MAX_LIGHTS];
uniform int uLightCount;

void main()
{
    // ---- Normal ----
    vec3 normal;
    if (uUseNormalMap)
    {
        vec3 tangentNormal = texture(uNormalMap, vUV).xyz * 2.0 - 1.0;
        normal = -normalize(vTBN * tangentNormal);
    }
    else
    {
        normal = -normalize(vNormal);
    }

    vec3 viewDir = normalize(uViewPos - vPos);

    vec3 ambient  = vec3(0.0);   // set to 0 for spotlight-only testing
    vec3 diffuse  = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < uLightCount; i++)
    {
        Light light = uLights[i];

        vec3 lightDir;
        float attenuation = 1.0;

        if (light.type == LIGHT_DIRECTIONAL)
        {
            lightDir = normalize(-light.direction);
        }
        else if (light.type == LIGHT_SPOT)
        {
            vec3 toLight = light.position - vPos;
            float dist = length(toLight);
            if (dist < 0.0001) continue;

            // Check if fragment is inside the spotlight cone
            vec3 fragToLight = normalize(toLight);
            float theta = dot(fragToLight, normalize(-light.direction));

            if (theta < light.cutoff)
            {
                continue;   // completely outside cone — no light at all
            }

            // Inside cone — smooth edge falloff
            float epsilon = 0.05;
            float edgeFade = clamp((theta - light.cutoff) / epsilon, 0.0, 1.0);

            // Spotlight uses its direction for lighting, position for distance
            lightDir = normalize(-light.direction);
            attenuation = edgeFade / (1.0 + (dist * dist) / (light.radius * light.radius));
        }
        else  // LIGHT_POINT
        {
            vec3 toLight = light.position - vPos;
            float dist = length(toLight);
            if (dist < 0.0001) continue;

            lightDir = toLight / dist;
            attenuation = 1.0 / (1.0 + (dist * dist) / (light.radius * light.radius));
        }

        // ---- Diffuse & Specular ----
        vec3 halfDir = normalize(lightDir + viewDir);
        float diff = max(dot(normal, lightDir), 0.0);
        float spec = pow(max(dot(normal, halfDir), 0.0), uShininess);

        vec3 diff_add = light.colour * diff * light.intensity * attenuation;
        vec3 spec_add = light.colour * spec * uSpecular * attenuation;

        diffuse  += diff_add;
        specular += spec_add;
    }

    vec3 lighting = ambient + diffuse + specular;

    vec4 finalColour = uColour;
    if (uUseVertexColour)
        finalColour *= vColour;

    vec4 texColour = vec4(1.0);
    if (uUseTexture)
        texColour = texture(uTexture, vUV);

    vec3 finalRGB = finalColour.rgb * texColour.rgb * lighting;
    finalRGB = pow(finalRGB, vec3(1.0 / 2.2));

    FragColour = vec4(finalRGB, finalColour.a * texColour.a);
}
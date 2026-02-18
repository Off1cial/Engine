#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include "types/types_vector.h"
#include <stdbool.h>

typedef struct {
    GLuint program;
    const char* vertexPath;
    const char* fragmentPath;


    // Cache

    GLint uModelLoc;
    GLint uViewLoc;
    GLint uProjLoc;
    GLint uViewPosLoc;
  
    GLint uUseTextureLoc;
    GLint uUseVertexColLoc;
    GLint uUseVertexColLoc_exclusive;
    GLint uTextureLoc;
    GLint uColourLoc;



} shader_t;


extern shader_t* SHADER_defaultShader;
extern shader_t* SHADER_defaultLit;
extern shader_t* SHADER_uiDefault;
extern shader_t* SHADER_defaultBillboard;
extern shader_t* SHADER_currentActive;
extern bool SHADER_isDefaultLoaded;
extern bool SHADER_isDefaultLitLoaded;
extern bool SHADER_isUIDefaultLoaded;
extern bool SHADER_isDefaultBillboardLoaded;

bool Shader_LoadDefaultShader(shader_t* defaultShader, const char* defaultVert, const char* defaultFrag);
bool Shader_LoadDefaultLit(shader_t* defaultLit, const char* litVert, const char* litFrag);
bool Shader_LoadDefaultUIShader(shader_t* uiShader, const char* uiVert, const char* uiFrag);

bool Shader_LoadDefaultBillboard(shader_t* billboardShader, const char* billboardVert, const char* billboardFrag);
bool Shader_Load(shader_t* shader_t, const char* vertexPath, const char* fragmentPath);
void Shader_Use(shader_t* shader_t);
void Shader_Destroy(shader_t* shader_t);

// Uniform setters
void Shader_SetMat4(shader_t* shader_t, const char* name, mat4 mat);
void Shader_SetVec4(shader_t* shader_t, const char* name, Vector4 vec);
void Shader_SetVec3(shader_t* shader_t, const char* name, Vector vec);
void Shader_SetFloat(shader_t* shader_t, const char* name, float value);
void Shader_SetInt(shader_t* shader_t, const char* name, int value);

#endif


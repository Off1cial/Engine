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


extern shader_t* SHADER_default_shader;
extern shader_t* SHADER_default_shader_lit;
extern shader_t* SHADER_default_shader_ui;
extern shader_t* SHADER_default_shader_billboard;
extern shader_t* SHADER_current_shader;
extern bool SHADER_isDefaultLoaded;
extern bool SHADER_isDefaultLitLoaded;
extern bool SHADER_isDefaultUILoaded;
extern bool SHADER_isDefaultBillboardLoaded;

bool Shader_LoadDefaultShader(shader_t* defaultShader, const char* defaultVert, const char* defaultFrag);
bool Shader_LoadDefaultLit(shader_t* defaultLit, const char* litVert, const char* litFrag);
bool Shader_LoadDefaultUIShader(shader_t* uiShader, const char* uiVert, const char* uiFrag);

bool Shader_LoadDefaultBillboard(shader_t* billboardShader, const char* billboardVert, const char* billboardFrag);
bool Shader_Load(shader_t* shader, const char* assetPath, const char* vertexPath, const char* fragmentPath);
void Shader_Use(shader_t* shader);
void Shader_Destroy(shader_t* shader);

// Uniform setters
void Shader_SetMat4(shader_t* shader, const char* name, mat4 mat);
void Shader_SetVec4(shader_t* shader, const char* name, Vector4 vec);
void Shader_SetVec3(shader_t* shader, const char* name, Vector vec);
void Shader_SetFloat(shader_t* shader, const char* name, float value);
void Shader_SetInt(shader_t* shader, const char* name, int value);

#endif


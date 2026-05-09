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


typedef struct {
  shader_t** shaders;
  size_t count;
  size_t capacity;
} shader_store_t;

void ShaderStore_Init(shader_store_t* store, size_t capacity);
long int ShaderStore_Add(shader_store_t* store, shader_t* shader);
void ShaderStore_Free(shader_store_t* store);


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


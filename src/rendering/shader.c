#include "rendering/shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool SHADER_isDefaultLoaded = false;
bool SHADER_isUIDefaultLoaded = false;
bool SHADER_isDefaultLitLoaded = false;
bool SHADER_isDefaultBillboardLoaded = false;

static char* ReadFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) { fprintf(stderr, "shader_t: failed to open %s\n", path); return NULL; }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) { fclose(file); return NULL; }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

static GLuint CompileShader(GLenum type, const char* src, const char* path) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(id, 1024, NULL, infoLog);
        fprintf(stderr, "[shader_t] %s compilation failed:\n%s\n", path, infoLog);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Shader_Load(shader_t* shader, const char* vertPath, const char* fragPath) {
    char* vertSrc = ReadFile(vertPath);
    char* fragSrc = ReadFile(fragPath);
    if (!vertSrc || !fragSrc) return false;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertSrc, vertPath);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragSrc, fragPath);

    free(vertSrc);
    free(fragSrc);

    if (!vs || !fs) return false;

    shader->program = glCreateProgram();
    glAttachShader(shader->program, vs);
    glAttachShader(shader->program, fs);
    glLinkProgram(shader->program);

    GLint success;
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(shader->program, 1024, NULL, infoLog);
        fprintf(stderr, "[shader_t] Linking failed:\n%s\n", infoLog);
        glDeleteProgram(shader->program);
        shader->program = 0;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    shader->uModelLoc = glGetUniformLocation(shader->program, "uModel");
    shader->uViewLoc = glGetUniformLocation(shader->program, "uView");
    shader->uProjLoc = glGetUniformLocation(shader->program, "uProj");
    shader->uColourLoc = glGetUniformLocation(shader->program, "uColour");
    shader->uTextureLoc = glGetUniformLocation(shader->program, "uTexture");
    shader->uUseTextureLoc = glGetUniformLocation(shader->program, "uUseTexture");
    shader->uUseVertexColLoc = glGetUniformLocation(shader->program, "uUseVertexCol");
    shader->uUseVertexColLoc_exclusive = glGetUniformLocation(shader->program, "uUseVertexCol_exclusive");

    shader->vertexPath = vertPath;
    shader->fragmentPath = fragPath;
    return true;
}


bool Shader_LoadDefaultShader(shader_t* defaultShader, const char* defaultVert, const char* defaultFrag){
  bool result = Shader_Load(defaultShader, defaultVert, defaultFrag);

  return result;
}

bool Shader_LoadDefaultLit(shader_t* defaultLit, const char* litVert, const char* litFrag){
  bool result = Shader_Load(defaultLit, litVert, litFrag);
  return result;
}

bool Shader_LoadDefaultUIShader(shader_t* uiShader, const char* uiVert, const char* uiFrag){
  bool result = Shader_Load(uiShader, uiVert, uiFrag);
  return result;
}

bool Shader_LoadDefaultBillboard(shader_t* billboardShader, const char* billboardVert, const char* billboardFrag){
  bool result = Shader_Load(billboardShader, billboardVert, billboardFrag);

  return result;
}

void Shader_Use(shader_t* shader) {
    glUseProgram(shader->program);
}

void Shader_Destroy(shader_t* shader) {
    glDeleteProgram(shader->program);
    shader->program = 0;
}

// ---- Uniform helpers ----

void Shader_SetMat4(shader_t* shader, const char* name, mat4 mat) {
  if (!shader){ fprintf(stderr, "ShaderSetMat4: shader_t is null\n"); }
    GLint loc = glGetUniformLocation(shader->program, name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mat.m);
}

void Shader_SetVec3(shader_t* shader, const char* name, Vector4 vec) {
    GLint loc = glGetUniformLocation(shader->program, name);
    if (loc != -1) glUniform3fv(loc, 1, vec.v);
}

void Shader_SetVec4(shader_t* shader, const char* name, Vector4 vec){
  GLint loc = glGetUniformLocation(shader->program, name);
  if (loc != -1) glUniform4fv(loc, 1, vec.v);
}

void Shader_SetFloat(shader_t* shader, const char* name, float value) {
    GLint loc = glGetUniformLocation(shader->program, name);
    if (loc != -1) glUniform1f(loc, value);
}

void Shader_SetInt(shader_t* shader, const char* name, int value) {
    GLint loc = glGetUniformLocation(shader->program, name);
    if (loc != -1) glUniform1i(loc, value);
}


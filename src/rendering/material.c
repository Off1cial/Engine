#include "rendering/material.h"
#include "tools/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



material_t* Material_Load(const char* filepath){

  char* text = Parser_ReadFileText(filepath);

  if (!text){
    fprintf(stderr, "[MATERIAL]: Unable to read materail file: %s\n", filepath);
    return NULL;
  }

  parser_t parser = {
    .at = text
  };

  char ident[128];

  material_t* m = calloc(1, sizeof(material_t));

  m->colour.x = 1.0f;
  m->colour.y = 0.0f;
  m->colour.z = 1.0f;
  m->colour.w = 1.0f;


  if (!m){
    fprintf(stderr, "[MATERIAL]: Unable to load material: %s\n", filepath);
    return NULL;
  }
  
  // Fill m with data from file
  // Parse here

  while (*parser.at){
    Parser_SkipWhitespace(&parser);
    if (!*parser.at){
      break;
    }

    // Handle braces explicitly
    if (*parser.at == '{' || *parser.at == '}'){
      parser.at++;
      continue;
    }

    // If it's not a valid identifier start, skip it
    if (!isalpha(*parser.at)){
      parser.at++;
      continue;
    }

    Parser_ReadIdentifier(&parser, ident);
    Parser_SkipWhitespace(&parser);

    if (*parser.at == '='){
      parser.at++; // Skip
    }
    Parser_SkipWhitespace(&parser); // Incase of whitespace after '='
    // Lit argument
    if (strcmp(ident, "Lit") == 0)
    {
      int lit = Parser_ReadBool(&parser);
      if (!lit){
        m->flags |= MATERIAL_UNLIT;
      }
    }

    // Texture
    else if (strcmp(ident, "Texture") == 0)
    {
      char texpath[256];
      Parser_ReadString(&parser, texpath);
      if (texpath[0] != '\0'){
        // Load texture
        texture_t* tex = Texture_Load(texpath);
        if (!tex){
          fprintf(stderr,
            "[MATERIAL]: Material texture failed to load\nMaterial: %s\nTexture: %s\n",
            filepath,
            texpath
          );
          m->base = NULL;
        }else{
          m->base = tex;
          m->flags |= MATERIAL_USE_TEXTURE;
        }
      }
    }

    // Use vertex colour
    else if (strcmp(ident, "UseVertexColour") == 0)
    {
      if (Parser_ReadBool(&parser)){
        m->flags |= MATERIAL_USE_VERTEX_COLOUR;
      }
    }
    // Colour
    else if (strcmp(ident, "Colour") == 0)
    {
      int r, g, b, a;
      if (sscanf(parser.at, "%d %d %d %d", &r, &g, &b, &a) == 4)
      {
        m->colour.x = r / 255.0f;
        m->colour.y = g / 255.0f;
        m->colour.z = b / 255.0f;
        m->colour.w = a / 255.0f;
      }
    }
    
    // Transparency
    else if (strcmp(ident, "Transparent") == 0){
      if (Parser_ReadBool(&parser)){
        m->flags |= MATERIAL_TRANSPARENT;
      }
    }
  }
  printf("[MATERIAL]: Finished parsing %s\n", filepath);




  printf("Lit:%d\n", ((m->flags & MATERIAL_UNLIT) == 0));

  printf("Use Texture: %d\n",
      (m->flags & MATERIAL_USE_TEXTURE) != 0);

  printf("Use Vertex Colour: %d\n",
      (m->flags & MATERIAL_USE_VERTEX_COLOUR) != 0);

  printf("Use Alpha: %d\n",
      (m->flags & MATERIAL_TRANSPARENT) != 0);


  free(text);

  return m;
}

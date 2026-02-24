#ifndef BRUSH_RENDER_H
#define BRUSH_RENDER_H

#include "mem.h"
#include "editor/brush.h"
#include "editorstate.h"
#include "rendering/mesh.h"
#include "rendering/draw_list.h"

#include "state.h"

// Not drawn in gameplay, just in editor
mesh_t* EditorBrush_CreateRenderMesh(brush_array_t* arr, size_t brush, struct mem_arena_t* arena);

// Assumes all meshes have already been created
void EditorBrush_DrawAll(editor_state_t* eState, state_t* state);



#endif

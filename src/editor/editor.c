#include "editor/editor.h"
#include "editor/editorcmd.h"

editor_cmd_queue_t* gEditorQueue = NULL;

void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext){
  
  EditorBrushArray_Init(gEditorBrushArray, 32);
  EditorGui_Init(window, glContext, eState->camera);
  
  editor_cmd_queue_t* q = malloc(sizeof(editor_cmd_queue_t));
  EditorQueue_Init(q, 32);
  gEditorQueue = q;
  
}

void EditorDestroy(editor_state_t* eState){
  EditorBrushArray_Destroy(gEditorBrushArray);
  free(gEditorBrushArray);
  free(eState);
}

void EditorToggle(SDL_Window* window){
  *gEditorActive = !(*gEditorActive);
  

  int winw, winh;
  SDL_GetWindowSizeInPixels(window, &winw, &winh);
  if (*gEditorActive){
    Camera_Switch(1, winh);
    RecalculatePanels(winw, winh, gCameras[1]);
  }else{
    Camera_Switch(0, winh);
  }
}

void EditorLoop(SDL_Window* window, rdrawqueue_t* draw_q, camera_t* editor_camera){
  EditorQueue_Reset(gEditorQueue);
  EditorGui_DrawAll(window, gInputState, editor_camera, gInputState->FLAG_WindowResized);
  EditorGui_HandleBrushInput(gInputState);
  EditorBrush_DrawAll(gEditorBrushArray, draw_q, gMemArena, editor_camera);
  EditorQueue_Execute(gEditorQueue, gEditorBrushArray);
}

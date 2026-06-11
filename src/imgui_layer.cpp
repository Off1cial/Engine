#include "imgui_layer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_opengl3.h>

void ImGui_PassEvent(SDL_Event* event){
  ImGui_ImplSDL3_ProcessEvent(event);
}

void ImGui_Init(SDL_Window* window, SDL_GLContext glContext){
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f;
  // Styling
  //ImGui::StyleColorsClassic();

  ImGui::StyleColorsDark();
  // Backend 
  ImGui_ImplSDL3_InitForOpenGL(window, glContext);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}

void ImGui_StartFrame(){
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void ImGui_Render(){
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGui_Shutdown()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

extern "C" bool ImGui_WantCaptureMouse(void)
{
    return ImGui::GetIO().WantCaptureMouse;
}
